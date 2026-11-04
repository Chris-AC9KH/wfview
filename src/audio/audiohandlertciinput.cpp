#include "audiohandlertciinput.h"
#include "tciserver.h"

bool audioHandlerTciInput::openDevice() noexcept {

    emit setupConverter(nativeFormat, codecType::LPCM, radioFormat, codec, 7,
                        setupData.resampleQuality);

    connect(this, &audioHandlerBase::sendToConverter, converter,
            &audioConverter::convert);
    connect(converter, SIGNAL(converted(audioPacket)), this,
            SLOT(onConverted(audioPacket)));
    if (setupData.tci != Q_NULLPTR) {
        // Audio from the TCI client (its TX stream) arrives here and goes
        // toward the radio.
        connect((tciServer *)setupData.tci, SIGNAL(sendTCIAudio(QByteArray)),
                this, SLOT(receiveTCIAudio(QByteArray)));
    } else {
        qCritical(logAudio()) << "***** TCI NOT FOUND *****";
    }
    inRB = std::make_unique<ByteRing>(
        nativeFormat.bytesForDuration(setupData.latency * 2000));

    return true;
}

void audioHandlerTciInput::closeDevice() noexcept {}

void audioHandlerTciInput::receiveTCIAudio(const QByteArray packet) {

    if (packet.size() == 0 || !inRB)
        return;
    inRB->push(static_cast<const char *>(packet), packet.size());
    QMetaObject::invokeMethod(
        this, [this] { onReadyRead(); }, Qt::QueuedConnection);
}

void audioHandlerTciInput::onReadyRead() {

    if (!inRB)
        return;
    QByteArray chunk;
    chunk.resize(static_cast<int>(inRB->size()));
    const size_t got =
        inRB->pop(chunk.data(), static_cast<size_t>(chunk.size()));
    if (!got)
        return;
    chunk.resize(static_cast<int>(got));

    tempBuf.data += chunk;

    const int bytesPerBlock =
        nativeFormat.bytesForDuration(setupData.blockSize * 1000);
    while (tempBuf.data.size() >= bytesPerBlock) {
        audioPacket pkt;
        pkt.time = QTime::currentTime();
        pkt.sent = 0;
        pkt.volume = volume;
        std::memcpy(&pkt.guid, setupData.guid, GUIDLEN);
        pkt.data = tempBuf.data.left(bytesPerBlock);
        tempBuf.data.remove(0, bytesPerBlock);
        emit sendToConverter(pkt);
    }
}

void audioHandlerTciInput::onConverted(audioPacket audio) {
    if (audio.data.isEmpty())
        return;

    emit haveAudioData(audio);

    if (lastReceived.isValid() && lastReceived.elapsed() > 100) {
        qDebug(logAudio()) << role() << "Time since last audio packet"
                           << lastReceived.elapsed() << "Expected around"
                           << setupData.blockSize;
    }
    if (!lastReceived.isValid())
        lastReceived.start();
    else
        lastReceived.restart();

    amplitude.store(audio.amplitudePeak, std::memory_order_relaxed);
    emit haveLevels(amplitudePeak(), audio.amplitudeRMS, setupData.latency,
                    static_cast<quint16>(latencyMs()), isUnderrun.load(),
                    isOverrun.load());
}

QAudioFormat audioHandlerTciInput::getNativeFormat() {
    QAudioFormat native;

    native.setSampleFormat(QAudioFormat::Float);
    native.setSampleRate(48000);
    native.setChannelCount(1);

    return native;
}

bool audioHandlerTciInput::isFormatSupported(QAudioFormat f) { return true; }
