#include "tciserver.h"
#include "logcategories.h"
#include <cmath>

// Updated based on TCI v2.0 / v2.1 commands
static const tciCommandStruct tci_commands[] = {
    // u=uchar s=short f=float b=bool x=hz m=mode s=string
    {"vfo_limits", funcNone, typeFreq, typeFreq, typeNone},
    {"if_limits", funcNone, typeFreq, typeFreq, typeNone},
    {"trx_count", funcNone, typeUChar, typeNone, typeNone},
    {"channel_count", funcNone, typeUChar, typeNone, typeNone},
    {"device", funcNone, typeShort, typeNone, typeNone},
    {"receive_only", funcNone, typeBinary, typeNone, typeNone},
    {"modulations_list", funcNone, typeShort, typeNone, typeNone},
    {"protocol", funcNone, typeShort, typeNone, typeNone},
    {"ready", funcNone, typeNone, typeNone, typeNone},
    {"start", funcNone, typeNone, typeNone, typeNone},
    {"stop", funcNone, typeNone, typeNone, typeNone},
    {"dds", funcNone, typeUChar, typeFreq, typeNone},
    {"if", funcNone, typeUChar, typeUChar, typeFreq},
    {"vfo", funcFreq, typeUChar, typeUChar, typeFreq},
    {"modulation", funcMode, typeUChar, typeMode, typeNone},
    {"trx", funcTransceiverStatus, typeUChar, typeBinary, typeNone},
    {"transceiver", funcTransceiverStatus, typeUChar, typeBinary,
     typeNone}, // TCI 2.0 normalization
    {"tune", funcTunerStatus, typeUChar, typeNone, typeNone},
    {"drive", funcRFPower, typeUShort, typeNone, typeNone},
    {"tx_power", funcRFPower, typeUShort, typeNone,
     typeNone}, // TCI 2.0 command
    {"tune_drive", funcNone, typeUChar, typeNone, typeNone},
    {"rit_enable", funcNone, typeUChar, typeNone, typeNone},
    {"xit_enable", funcNone, typeUChar, typeNone, typeNone},
    {"split_enable", funcSplitStatus, typeUChar, typeBinary, typeNone},
    {"rit_offset", funcNone, typeUChar, typeNone, typeNone},
    {"xit_offset", funcNone, typeUChar, typeNone, typeNone},
    {"rx_channel_enable", funcNone, typeUChar, typeNone, typeNone},
    {"rx_filter_band", funcNone, typeUChar, typeNone, typeNone},
    {"rx_filter_fl", funcNone, typeUChar, typeShort,
     typeShort}, // TCI 2.0 command
    {"cw_macros_speed", funcNone, typeUChar, typeNone, typeNone},
    {"cw_macros_delay", funcNone, typeUChar, typeNone, typeNone},
    {"cw_keyer_speed", funcKeySpeed, typeUChar, typeNone, typeNone},
    {"volume", funcAfGain, typeUChar, typeNone, typeNone},
    {"mute", funcNone, typeUChar, typeNone, typeNone},
    {"rx_mute", funcNone, typeUChar, typeNone, typeNone},
    {"rx_volume", funcNone, typeUChar, typeNone, typeNone},
    {"rx_balance", funcNone, typeUChar, typeNone, typeNone},
    {"mon_volume", funcNone, typeUChar, typeNone, typeNone},
    {"mon_enable", funcNone, typeUChar, typeNone, typeNone},
    {"agc_mode", funcNone, typeUChar, typeNone, typeNone},
    {"agc_gain", funcNone, typeUChar, typeNone, typeNone},
    {"rx_nb_enable", funcNone, typeUChar, typeNone, typeNone},
    {"rx_nb_param", funcNone, typeUChar, typeNone, typeNone},
    {"rx_bin_enable", funcNone, typeUChar, typeNone, typeNone},
    {"rx_nr_enable", funcNone, typeUChar, typeNone, typeNone},
    {"rx_anc_enable", funcNone, typeUChar, typeNone, typeNone},
    {"rx_anf_enable", funcNone, typeUChar, typeNone, typeNone},
    {"rx_apf_enable", funcNone, typeUChar, typeNone, typeNone},
    {"rx_dse_enable", funcNone, typeUChar, typeNone, typeNone},
    {"rx_nf_enable", funcNone, typeUChar, typeNone, typeNone},
    {"lock", funcNone, typeUChar, typeNone, typeNone},
    {"sql_enable", funcNone, typeUChar, typeNone, typeNone},
    {"sql_level", funcSquelch, typeUChar, typeNone, typeNone},
    {"tx_enable", funcNone, typeUChar, typeUChar, typeNone},
    {"cw_macros_speed_up", funcNone, typeUChar, typeUChar, typeNone},
    {"cw_macros_speed_down", funcNone, typeUChar, typeUChar, typeNone},
    {"cw_msg", funcNone, typeUChar, typeString, typeNone},    // TCI 2.0 command
    {"cw_msg_stop", funcNone, typeUChar, typeNone, typeNone}, // TCI 2.0 command
    {"spot", funcNone, typeUChar, typeUChar, typeNone},
    {"spot_delete", funcNone, typeUChar, typeUChar, typeNone},
    {"iq_samplerate", funcNone, typeUChar, typeNone, typeNone},
    {"audio_samplerate", funcNone, typeUChar, typeNone, typeNone},
    {"iq_start", funcNone, typeUChar, typeNone, typeNone},
    {"iq_stop", funcNone, typeUChar, typeNone, typeNone},
    {"audio_start", funcNone, typeUChar, typeNone, typeNone},
    {"audio_stop", funcNone, typeUChar, typeNone, typeNone},
    {"tx_audio_start", funcNone, typeNone, typeNone,
     typeNone}, // TCI 2.0 command
    {"tx_audio_stop", funcNone, typeNone, typeNone,
     typeNone}, // TCI 2.0 command
    {"line_out_start", funcNone, typeUChar, typeNone, typeNone},
    {"line_out_stop", funcNone, typeUChar, typeNone, typeNone},
    {"line_out_recorder_start", funcNone, typeUChar, typeNone, typeNone},
    {"line_out_recorder_save", funcNone, typeUChar, typeNone, typeNone},
    {"clicked_on_spot", funcNone, typeUChar, typeNone, typeNone},
    {"rx_clicked_on_spot", funcNone, typeUChar, typeNone, typeNone},
    {"tx_footswitch", funcNone, typeUChar, typeBinary, typeNone},
    {"tx_frequency", funcNone, typeUChar, typeBinary, typeNone},
    {"app_focus", funcNone, typeUChar, typeBinary, typeNone},
    {"set_in_focus", funcNone, typeUChar, typeBinary, typeNone},
    {"keyer", funcNone, typeUChar, typeBinary, typeNone},
    {"rx_sensors_enable", funcNone, typeUChar, typeBinary, typeNone},
    {"tx_sensors_enable", funcNone, typeUChar, typeBinary, typeNone},
    {"rx_sensors", funcNone, typeUChar, typeBinary, typeNone},
    {"tx_sensors", funcNone, typeUChar, typeBinary, typeNone},
    {"audio_stream_sample_type", funcNone, typeUChar, typeBinary, typeNone},
    {"audio_stream_channels", funcNone, typeUChar, typeBinary, typeNone},
    {"audio_stream_samples", funcNone, typeUChar, typeBinary, typeNone},
    {"digl_offset", funcNone, typeUChar, typeBinary, typeNone},
    {"digu_offset", funcNone, typeUChar, typeBinary, typeNone},
    {"rx_smeter", funcSMeter, typeUChar, typeUChar, typeDouble},
    {0x0, funcNone, typeNone, typeNone, typeNone},
};

#define MAXNAMESIZE 32
tciServer::tciServer(QObject *parent) : QObject(parent) {}

void tciServer::init(quint16 port) {
    server = new QWebSocketServer(QStringLiteral("TCI Server"),
                                  QWebSocketServer::NonSecureMode, this);

    if (server->listen(QHostAddress::Any, port)) {
        qInfo() << "tciServer() listening on" << port;
        connect(server, &QWebSocketServer::newConnection, this,
                &tciServer::onNewConnection);
        connect(server, &QWebSocketServer::closed, this,
                &tciServer::socketDisconnected);
    }

    this->setObjectName("TCI Server");
    queue = cachingQueue::getInstance();
    rigCaps = queue->getRigCaps();

    connect(queue, SIGNAL(rigCapsUpdated(rigCapabilities *)), this,
            SLOT(receiveRigCaps(rigCapabilities *)));
    connect(queue, SIGNAL(cacheUpdated(cacheItem)), this,
            SLOT(receiveCache(cacheItem)));

    // Setup txChrono packet.
    txChrono.resize(iqHeaderSize);
    txChrono.fill(0);
    dataStream *pStream = reinterpret_cast<dataStream *>(txChrono.data());
    pStream->receiver = 0;
    pStream->sampleRate = 48000;
    pStream->format = IqFloat32;
    pStream->codec = 0u;
    pStream->type = TxChrono;
    pStream->crc = 0u;
    pStream->length = TCI_AUDIO_LENGTH;
    pStream->reserv[0] = 1; // channels (header word 7 in TCI 2.1)
}

void tciServer::updateAudioState() {
    bool any = false;
    for (auto it = clients.constBegin(); it != clients.constEnd(); ++it) {
        if (it.value().connected && it.value().rxaudio) {
            any = true;
            break;
        }
    }
    if (any != audioActive) {
        audioActive = any;
        qInfo() << "TCI audio active:" << any;
        emit audioActiveChanged(any);
    }
}

void tciServer::audioStreamCommand(QWebSocket *client, connStatus &c, const QString &cmd, const QString &value)
{
    static const char *const typeNames[] = { "int16", "int24", "int32", "float32" };
    const QString v = value.trimmed().toLower();
    bool ok = false;

    if (cmd == "audio_stream_sample_type")
    {
        // An unknown format is ignored; the reply always carries the format in effect.
        if (v == "int16") c.sampleType = IqInt16;
        else if (v == "int24") c.sampleType = IqInt24;
        else if (v == "int32") c.sampleType = IqInt32;
        else if (v == "float32") c.sampleType = IqFloat32;
        client->sendTextMessage(QString("audio_stream_sample_type:%0;").arg(typeNames[c.sampleType]));
    }
    else if (cmd == "audio_stream_channels")
    {
        const quint32 n = v.toUInt(&ok);
        if (ok) c.channels = qBound(1u, n, 2u);       // values outside 1..2 are limited
        client->sendTextMessage(QString("audio_stream_channels:%0;").arg(c.channels));
    }
    else if (cmd == "audio_stream_samples")
    {
        const quint32 n = v.toUInt(&ok);
        if (ok) c.samples = qBound(100u, n, 2048u);   // spec range 100..2048
        client->sendTextMessage(QString("audio_stream_samples:%0;").arg(c.samples));
    }

    qInfo() << "TCI audio stream format:" << typeNames[c.sampleType]
            << "channels" << c.channels << "samples" << c.samples;
}

void tciServer::setupTxPacket(int length) { Q_UNUSED(length) }

tciServer::~tciServer() {
    server->close();
    auto it = clients.begin();
    while (it != clients.end()) {
        it.key()->deleteLater();
        it = clients.erase(it);
    }
}

void tciServer::receiveRigCaps(rigCapabilities *caps) { rigCaps = caps; }

int tciServer::getValueRange(funcs func, char min, char max, uchar rx) {
    int val = min;
    if (rigCaps->commands.contains(func)) {
        auto cmd = rigCaps->commands.find(func);
        float r =
            min +
            (float(queue->getCache(func, rx).value.toInt() - cmd->minVal) *
             (max - min)) /
                (float)(cmd->maxVal - cmd->minVal);
        val = static_cast<int>(r + (r < 0 ? -0.5f : 0.5f));
    }
    return val;
}
void tciServer::onNewConnection() {

    QWebSocket *pSocket = server->nextPendingConnection();
    if (!pSocket)
        return;

    vfoCommandType t = queue->getVfoCommand(vfoA, false, false);

    if (rigCaps == Q_NULLPTR) {
        qWarning(logTCIServer())
            << "No current rig connection, denying connection request.";
        pSocket->abort();
        return;
    }

    qInfo(logTCIServer()) << "Got rigCaps for" << rigCaps->modelName;

    connect(pSocket, &QWebSocket::textMessageReceived, this,
            &tciServer::processIncomingTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this,
            &tciServer::processIncomingBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this,
            &tciServer::socketDisconnected);

    qInfo() << "TCI client connected:" << pSocket;
    clients.insert(pSocket, connStatus());

    // TCI v2.0 Protocol handshake
    pSocket->sendTextMessage(QString("protocol:WFVIEW,2.0;"));
    pSocket->sendTextMessage(QString("device:%0;").arg(rigCaps->modelName));
    pSocket->sendTextMessage(QString("receive_only:%0;")
                                 .arg(rigCaps->hasTransmit ? "false" : "true"));
    pSocket->sendTextMessage(
        QString("trx_count:%0;").arg(rigCaps->numReceiver));
    pSocket->sendTextMessage(
        QString("channels_count:%0;").arg(rigCaps->numVFO));

    quint64 start = UINT64_MAX;
    quint64 end = 0;
    for (auto &band : rigCaps->bands) {
        if (start > band.lowFreq)
            start = band.lowFreq;
        if (end < band.highFreq)
            end = band.highFreq;
    }
    pSocket->sendTextMessage(QString("vfo_limits:%0,%1;").arg(start).arg(end));
    pSocket->sendTextMessage(QString("if_limits:-19531,19531;"));
    QString mods = "modulations_list:";
    for (modeInfo &mi : rigCaps->modes) {
        mods += mi.name.toUpper();
        mods += ",";
        if (mi.reg == modeUSB || mi.reg == modeLSB) {
            mods += QString("DIG%0").arg(mi.name.at(0));
            mods += ",";
        }
    }
    mods.chop(1);
    mods += ";";
    pSocket->sendTextMessage(mods);

    pSocket->sendTextMessage(QString("vfo:0,0,%0;")
                                 .arg(queue->getCache(t.freqFunc, t.receiver)
                                          .value.value<freqt>()
                                          .Hz));
    pSocket->sendTextMessage(
        QString("modulation:0,%0;")
            .arg(tciMode(queue->getCache(t.modeFunc, t.receiver)
                             .value.value<modeInfo>())));

    // TCI 2.0 stream configuration prerequisites
    pSocket->sendTextMessage(QString("audio_stream_sample_type:float32;"));
    pSocket->sendTextMessage(QString("audio_stream_channels:1;"));
    pSocket->sendTextMessage(QString("iq_samplerate:48000;"));
    pSocket->sendTextMessage(QString("audio_samplerate:48000;"));

    pSocket->sendTextMessage(
        QString("volume:%0;")
            .arg(getValueRange(funcAfGain, -60, 0, t.receiver)));
    pSocket->sendTextMessage(
        QString("mute:%0;")
            .arg(queue->getCache(funcAFMute, t.receiver).value.value<bool>()
                     ? "true"
                     : "false"));
    pSocket->sendTextMessage(
        QString("mon_volume:%0;")
            .arg(getValueRange(funcMonitorGain, -60, 0, t.receiver)));
    pSocket->sendTextMessage(
        QString("mon_enable:%0;")
            .arg(queue->getCache(funcMonitor, t.receiver).value.value<bool>()
                     ? "false"
                     : "true"));

    pSocket->sendTextMessage(QString("ready;"));
}

void tciServer::processIncomingTextMessage(QString message) {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (!pClient)
        return;

    auto it = clients.find(pClient);
    if (it == clients.end()) {
        qInfo() << "Cannot find status for connection" << pClient;
        return;
    }

    it.value().connected = true;
    qDebug() << "TCI Text Message received:" << message;

    QString cmd = message.section(':', 0, 0).toLower().trimmed();
    QString rest = message.section(':', 1, 1);
    if (rest.endsWith(';'))
        rest.chop(1);
    QStringList arg = rest.split(',', Qt::SkipEmptyParts);

    QString reply = message;
    
    QString name = cmd;
    if (name.endsWith(';')) name.chop(1);   // commands with no arguments keep their ';' in cmd
    if (name == "audio_stream_sample_type" ||
        name == "audio_stream_channels" ||
        name == "audio_stream_samples")
    {
        audioStreamCommand(pClient, it.value(), name, rest);
        return;
    }

    uchar rx = 0;
    if (arg.size() > 0) {
        rx = arg[0].toUInt();
    }
    uchar vfo = 0;
    if (arg.size() > 1) {
        vfo = arg[1].toUInt();
    }
    bool set = false;

    for (int i = 0; tci_commands[i].str != 0x00; i++) {
        if (!strncmp(cmd.toLocal8Bit().constData(), tci_commands[i].str,
                     MAXNAMESIZE)) {
            tciCommandStruct tc = tci_commands[i];
            uchar numArgs = 0;
            if (tc.arg1 != typeNone)
                numArgs++;
            if (tc.arg2 != typeNone)
                numArgs++;
            if (tc.arg3 != typeNone)
                numArgs++;

            if (numArgs <= arg.count() && !rest.isEmpty()) {
                set = true;
            }

            if (vfo && rigCaps->numVFO <= vfo && rigCaps->numReceiver > 1) {
                rx = vfo;
                vfo = 0;
            }
            vfoCommandType t = queue->getVfoCommand(vfo_t(vfo), rx, set);
            funcs func = tc.func;

            if (set) {
                QVariant val;
                if (arg.count() == 1 && tc.arg1 == typeUChar) {
                    val = QVariant::fromValue(uchar(arg[0].toInt(NULL)));
                }
                if (arg.count() == 1 && tc.arg1 == typeUShort) {
                    val =
                        QVariant::fromValue(ushort(arg[0].toInt(NULL) * 2.55));
                }
                if (arg.count() == 2 && tc.arg2 == typeUChar) {
                    val = QVariant::fromValue(uchar(arg[1].toInt(NULL)));
                }
                if (arg.count() == 2 && tc.arg2 == typeUShort) {
                    val =
                        QVariant::fromValue(ushort(arg[1].toInt(NULL) * 2.55));
                } else if (arg.count() == 3 && tc.arg3 == typeFreq) {
                    val = QVariant::fromValue<freqt>(
                        freqt(arg[2].toUInt(), arg[2].toUInt() / (double)1E6,
                              selVFO_t::activeVFO));
                    func = t.freqFunc;
                } else if (tc.arg2 == typeMode) {
                    val = QVariant::fromValue<modeInfo>(rigMode(arg[1]));
                    func = t.modeFunc;
                } else if (tc.arg2 == typeBinary) {
                    val = QVariant::fromValue(
                        arg[1].compare("true", Qt::CaseInsensitive) == 0 ||
                        arg[1] == "1");
                }

                if (func != funcNone && val.isValid()) {
                    queue->add(priorityImmediate,
                               queueItem(func, val, false, rx));
                }
            }
            if (cmd == "audio_start" || cmd == "tx_audio_start") {
                qInfo() << "Starting TCI audio stream:" << cmd;
                if (cmd == "audio_start") {
                    it.value().rxaudio = true;
                    reply = QString("audio_start:%0;").arg(rx);
                } else {
                    it.value().txaudio = true;
                    reply = "tx_audio_start;";
                }
                updateAudioState();
                pClient->sendTextMessage(reply);
                return;
            } else if (cmd == "audio_stop" || cmd == "tx_audio_stop") {
                qInfo() << "Stopping TCI audio stream:" << cmd;
                if (cmd == "audio_stop") {
                    it.value().rxaudio = false;
                    reply = QString("audio_stop:%0;").arg(rx);
                } else {
                    it.value().txaudio = false;
                    reply = "tx_audio_stop;";
                }
                updateAudioState();
                pClient->sendTextMessage(reply);
                return;
            }

            if (tc.func != funcNone) {
                reply = QString("%0").arg(cmd);
                QVariant v = queue->getCache(func, t.receiver).value;
                if (!v.isValid()) {
                    continue;
                } else {
                    if (tc.arg3 == typeFreq) {
                        v = queue->getCache(t.freqFunc, t.receiver).value;
                    }
                    if (tc.arg3 == typeMode) {
                        v = queue->getCache(t.modeFunc, t.receiver).value;
                    }
                }

                if (tc.arg1 == typeUChar && numArgs > 1) {
                    reply += QString(":%0").arg(arg[0]);
                }

                if (numArgs == 1 && tc.arg1 == typeUChar) {
                    reply += QString(":%0").arg(v.value<uchar>());
                } else if (numArgs == 1 && tc.arg1 == typeUShort) {
                    reply += QString(":%0").arg(
                        std::round(v.value<ushort>() / 2.55));
                } else if (numArgs == 2 && tc.arg2 == typeUChar) {
                    reply += QString(",%0").arg(v.value<uchar>());
                } else if (numArgs == 2 && tc.arg2 == typeUShort) {
                    reply += QString(",%0").arg(
                        std::round(v.value<ushort>() / 2.55));
                } else if (numArgs == 3 && tc.arg3 == typeFreq) {
                    if (rigCaps->numReceiver > 1 && t.receiver > 0) {
                        vfo = t.receiver;
                    }
                    reply += QString(",%0,%1").arg(vfo).arg(
                        static_cast<quint64>(v.value<freqt>().Hz));
                } else if (numArgs == 3 && tc.arg3 == typeDouble) {
                    reply += QString(",%0,%1").arg(vfo).arg(
                        v.value<double>() - dBmConversion, 0, 'f', 0);
                } else if (tc.arg2 == typeMode) {
                    reply += QString(",%0").arg(tciMode(v.value<modeInfo>()));
                } else if (tc.arg2 == typeBinary) {
                    reply +=
                        QString(",%0").arg(v.value<bool>() ? "true" : "false");
                } else {
                    return;
                }
                reply += ";";
            } else {
                if (!reply.endsWith(';'))
                    reply += ";";
            }

            if (pClient) {
                pClient->sendTextMessage(reply);
            }
            return;
        }
    }
}

void tciServer::processIncomingBinaryMessage(QByteArray message) {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (!pClient || message.size() < qsizetype(iqHeaderSize))
        return;

    const dataStream *pStream =
        reinterpret_cast<const dataStream *>(message.constData());
    if (pStream->type != TxAudioStream || pStream->length == 0)
        return;

    QByteArray tempData;

    if (pStream->format == IqFloat32) {
        // --- Native Float32 Path ---
        const qsizetype available =
            (message.size() - qsizetype(iqHeaderSize)) / qsizetype(sizeof(float));
        const qsizetype count = qMin<qsizetype>(pStream->length, available);
        tempData.resize(count * qsizetype(sizeof(float)), 0x0);
        memcpy(tempData.data(), message.constData() + iqHeaderSize, tempData.size());
    }
    else if (pStream->format == IqInt16) {
        // --- Native ExpertSDR3 Compliant Int16 Path ---
        const qsizetype available =
            (message.size() - qsizetype(iqHeaderSize)) / qsizetype(sizeof(int16_t));
        const qsizetype count = qMin<qsizetype>(pStream->length, available);
        
        // Allocate space matching downstream float requirements
        tempData.resize(count * qsizetype(sizeof(float)), 0x0);
        float *outFloats = reinterpret_cast<float *>(tempData.data());
        const int16_t *inInts = reinterpret_cast<const int16_t *>(message.constData() + iqHeaderSize);
        
        // De-interleave and unpack raw integer samples to full-scale float [-1.0f, 1.0f]
        for (qsizetype i = 0; i < count; ++i) {
            outFloats[i] = static_cast<float>(inInts[i]) / 32678.0f;
        }
    }
    else {
        // If an unhandled format type arrives, ignore per protocol spec
        return;
    }

    // Pass the parsed network audio frame to the radio-hardware bridge loop
    emit sendTCIAudio(tempData);
}

void tciServer::socketDisconnected() {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qInfo() << "TCI client disconnected:" << pClient;
    if (pClient) {
        clients.remove(pClient);
        updateAudioState();
        pClient->deleteLater();
    }
}

void tciServer::receiveTCIAudio(audioPacket audio) {
    if (audio.data.isEmpty())
        return;

    static quint32 rxCount = 0;
    const qsizetype bytes =
        qMin<qsizetype>(audio.data.size(), TCI_AUDIO_LENGTH * sizeof(float));

    rxAudioData.resize(iqHeaderSize + bytes);
    std::memset(rxAudioData.data(), 0, iqHeaderSize);
    dataStream *pStream = reinterpret_cast<dataStream *>(rxAudioData.data());

    pStream->receiver = 0;
    pStream->sampleRate = 48000;
    pStream->format = IqFloat32;
    pStream->codec = 0u;
    pStream->type = RxAudioStream;
    pStream->crc = 0u;
    pStream->length = quint32(bytes / sizeof(float));
    pStream->reserv[0] = 1;         // channels (header word 7 in TCI 2.1)
    pStream->reserv[1] = rxCount++; // packet counter (header word 8 in TCI 2.1)

    std::memcpy(pStream->data, audio.data.constData(), bytes);

    auto it = clients.begin();
    while (it != clients.end()) {
        if (it.value().connected && it.value().rxaudio) {
            it.key()->sendBinaryMessage(txChrono);
            it.key()->sendBinaryMessage(rxAudioData);
        }
        ++it;
    }
}

void tciServer::receiveCache(cacheItem item) {
    auto it = clients.begin();
    QString reply;

    funcs func = item.command;
    uchar vfo = 0;
    if (func == funcModeTR || func == funcSelectedMode) {
        func = funcMode;
    } else if (func == funcFreqTR || func == funcSelectedFreq) {
        func = funcFreq;
    } else if (func == funcUnselectedMode) {
        func = funcMode;
        vfo = 1;
    } else if (func == funcUnselectedFreq) {
        func = funcFreq;
        vfo = 1;
    }

    while (it != clients.end()) {
        if (it.value().connected) {
            for (int i = 0; tci_commands[i].str != 0x00; i++) {
                if (func == tci_commands[i].func) {
                    tciCommandStruct tc = tci_commands[i];
                    uchar numArgs = 0;
                    if (tc.arg1 != typeNone)
                        numArgs++;
                    if (tc.arg2 != typeNone)
                        numArgs++;
                    if (tc.arg3 != typeNone)
                        numArgs++;

                    if (tc.func != funcNone) {
                        if (tc.arg1 == typeUChar && numArgs > 1)
                            reply = QString("%0:%1").arg(tc.str).arg(
                                QString::number(item.receiver));
                        else if (numArgs == 1 && tc.arg1 == typeUChar)
                            reply = QString("%0:%1").arg(tc.str).arg(
                                item.value.value<uchar>());
                        else if (numArgs == 1 && tc.arg1 == typeUShort)
                            reply = QString("%0:%1").arg(tc.str).arg(
                                std::round(item.value.value<ushort>() / 2.55));

                        if (numArgs == 2 && tc.arg2 == typeUChar) {
                            reply +=
                                QString(",%0").arg(item.value.value<uchar>());
                        } else if (numArgs == 2 && tc.arg2 == typeUShort) {
                            reply += QString(",%0").arg(
                                std::round(item.value.value<ushort>() / 2.55));
                        } else if (numArgs == 3 && tc.arg3 == typeDouble) {
                            reply += QString(",%0,%1").arg(vfo).arg(
                                item.value.value<double>() - dBmConversion, 0,
                                'f', 0);
                        } else if (numArgs == 3 && tc.arg3 == typeFreq) {
                            if (rigCaps->numReceiver > 1 && item.receiver > 0) {
                                vfo = item.receiver;
                            }
                            reply += QString(",%0,%1").arg(vfo).arg(
                                static_cast<quint64>(
                                    item.value.value<freqt>().Hz));

                            if (item.value.value<freqt>().Hz > 30000000) {
                                dBmConversion = 93;
                            } else {
                                dBmConversion = 73;
                            }
                        } else if (tc.arg2 == typeMode) {
                            reply += QString(",%0").arg(
                                tciMode(item.value.value<modeInfo>()));
                        } else if (tc.arg2 == typeBinary) {
                            reply += QString(",%0").arg(
                                item.value.value<bool>() ? "true" : "false");
                        }
                        reply += ";";
                        it.key()->sendTextMessage(reply);
                    }
                }
            }
        }
        ++it;
    }
}

QString tciServer::tciMode(modeInfo m) {
    QString ret = "";
    if (m.mk == modeUSB && m.data && m.data != 0xff)
        ret = "digu";
    else if (m.mk == modeLSB && m.data != 0xff)
        ret = "digl";
    else if (m.mk == modeFM)
        ret = "nfm";
    else
        ret = m.name.toLower();
    return ret;
}

modeInfo tciServer::rigMode(QString mode) {
    modeInfo m;
    m.mk = modeUnknown;
    qInfo() << "Searching for mode" << mode;
    for (modeInfo &mi : rigCaps->modes) {
        if (mode.toLower() == "digl" && mi.mk == modeLSB) {
            m = modeInfo(mi);
            m.data = true;
            break;
        } else if (mode.toLower() == "digu" && mi.mk == modeUSB) {
            m = modeInfo(mi);
            m.data = true;
            break;
        } else if ((mode.toLower() == "nfm" && mi.mk == modeFM) ||
                   (mode.toLower() == mi.name.toLower())) {
            m = modeInfo(mi);
            m.data = false;
            break;
        }
    }
    m.filter = 1;
    qInfo(logTCIServer()) << "Got Mode:" << m.name << "data:" << m.data;
    return m;
}
