#include "tciaudiobridge.h"
#include "logcategories.h"

tciAudioBridge::tciAudioBridge(QObject *parent) : QObject(parent) {}

tciAudioBridge::~tciAudioBridge() { stop(); }

audioHandlerBase *tciAudioBridge::makeInput(audioType type) {
    switch (type) {
    case qtAudio:
        return new audioHandlerQtInput();
    case portAudio:
        return new audioHandlerPaInput();
    case rtAudio:
        return new audioHandlerRtInput();
    default:
        return nullptr;
    }
}

audioHandlerBase *tciAudioBridge::makeOutput(audioType type) {
    switch (type) {
    case qtAudio:
        return new audioHandlerQtOutput();
    case portAudio:
        return new audioHandlerPaOutput();
    case rtAudio:
        return new audioHandlerRtOutput();
    default:
        return nullptr;
    }
}

void tciAudioBridge::place(audioHandlerBase *handler, const QString &name) {
    QThread *thread = new QThread();
    thread->setObjectName(name);
    handler->moveToThread(thread);
    connect(thread, &QThread::finished, handler, &QObject::deleteLater);
    thread->start(QThread::TimeCriticalPriority);
    threads.append(thread);
}

void tciAudioBridge::initHandler(audioHandlerBase *handler,
                                 const audioSetup &setup) {
    QMetaObject::invokeMethod(
        handler, [handler, setup]() { handler->init(setup); },
        Qt::QueuedConnection);
}

void tciAudioBridge::start(const audioSetup &rxDevice,
                           const audioSetup &txDevice, void *tci) {
    if (running || tci == nullptr) {
        return;
    }

    // Radio-side format is always 48 kHz, mono, 16-bit (codec 0x04).
    auto prepare = [](audioSetup s, bool isInput) {
        s.codec = 0x04;
        s.sampleRate = 48000;
        s.latency = 150;
        s.localAFgain = 255; // audiopot[0] is silence; the field has no default
        s.resampleQuality = 4;
        s.isinput = isInput;
        s.txProc = nullptr;
        s.rxProc = nullptr;
        return s;
    };

    const audioSetup capture = prepare(rxDevice, true);
    const audioSetup playback = prepare(txDevice, false);

    audioSetup toTci = prepare(rxDevice, false);
    toTci.type = tciAudio;
    toTci.tci = tci;
    toTci.portInt =
        0; // TCI handlers have no device, but init() rejects portInt == -1

    audioSetup fromTci = prepare(txDevice, true);
    fromTci.type = tciAudio;
    fromTci.tci = tci;
    fromTci.portInt = 0;

    rxCapture = makeInput(capture.type);
    txPlayback = makeOutput(playback.type);
    if (!rxCapture || !txPlayback) {
        qCritical(logAudio()) << "TCI audio bridge: unsupported audio system "
                                 "for the radio's sound card";
        delete rxCapture;
        delete txPlayback;
        rxCapture = txPlayback = nullptr;
        return;
    }
    rxToTci = new audioHandlerTciOutput();
    txFromTci = new audioHandlerTciInput();

    place(rxCapture, QStringLiteral("tciRxCapture()"));
    place(rxToTci, QStringLiteral("tciRxToTci()"));
    place(txFromTci, QStringLiteral("tciTxFromTci()"));
    place(txPlayback, QStringLiteral("tciTxPlayback()"));

    // Connect before init so no packet is missed.
    connect(rxCapture, SIGNAL(haveAudioData(audioPacket)), rxToTci,
            SLOT(incomingAudio(audioPacket)));
    connect(txFromTci, SIGNAL(haveAudioData(audioPacket)), txPlayback,
            SLOT(incomingAudio(audioPacket)));

    // Consumers first, then producers.
    initHandler(rxToTci, toTci);
    initHandler(txPlayback, playback);
    initHandler(txFromTci, fromTci);
    initHandler(rxCapture, capture);

    running = true;
    qInfo(logAudio()) << "TCI audio bridge started, device:" << capture.name
                      << "/" << playback.name;
}

void tciAudioBridge::stop() {
    if (!running && threads.isEmpty()) {
        return;
    }

    // dispose() marshals itself onto each handler's own thread.
    for (audioHandlerBase *h : {rxCapture, txFromTci, rxToTci, txPlayback}) {
        if (h) {
            h->dispose();
        }
    }
    for (QThread *t : threads) {
        t->quit();
        t->wait();
        t->deleteLater();
    }
    threads.clear();
    rxCapture = rxToTci = txFromTci = txPlayback = nullptr;
    running = false;
    qInfo(logAudio()) << "TCI audio bridge stopped";
}
