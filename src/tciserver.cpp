#include "tciserver.h"
#include "logcategories.h"
#include <cmath>
#include <QTimer>
#include <QElapsedTimer>
#include <QPointer>
#include <QDateTime>

// Updated based on TCI v2.0 / v2.1 commands
static const tciCommandStruct tci_commands[] = {
    // u=uchar s=short f=float b=bool x=hz m=mode s=string
    {"vfo_limits", funcNone, typeFreq, typeFreq, typeNone},
    {"if_limits", funcNone, typeFreq, typeFreq, typeNone},
    {"trx_count", funcNone, typeUChar, typeNone, typeNone},
    {"channel_count", funcNone, typeUChar, typeNone, typeNone},
    {"channels_count", funcNone, typeUChar, typeNone, typeNone},
    {"device", funcNone, typeShort, typeNone, typeNone},
    {"receive_only", funcNone, typeBinary, typeNone, typeNone},
    {"modulations_list", funcNone, typeShort, typeNone, typeNone},
    {"protocol", funcNone, typeShort, typeNone, typeNone},
    {"ready", funcNone, typeNone, typeNone, typeNone},
    {"start", funcNone, typeNone, typeNone, typeNone},
    {"stop", funcNone, typeNone, typeNone, typeNone},
    {"dds", funcNone, typeUChar, typeFreq, typeNone},
    {"if", funcNone, typeUChar, typeUChar, typeFreq},
    {"rx_filter_fl", funcNone, typeUChar, typeShort,
     typeShort}, // TCI 2.0 command (inbound only)
    {"vfo", funcFreq, typeUChar, typeUChar, typeFreq},
    {"modulation", funcMode, typeUChar, typeMode, typeNone},
    {"trx", funcTransceiverStatus, typeUChar, typeBinary, typeNone},
    {"tune", funcTunerStatus, typeUChar, typeNone, typeNone},
    {"drive", funcRFPower, typeUChar, typeUShort, typeNone}, // DRIVE:trx,0..100
    {"tune_drive", funcNone, typeUChar, typeNone, typeNone},
    {"rit_enable", funcNone, typeUChar, typeNone, typeNone},
    {"xit_enable", funcNone, typeUChar, typeNone, typeNone},
    {"split_enable", funcSplitStatus, typeUChar, typeBinary, typeNone},
    {"rit_offset", funcNone, typeUChar, typeNone, typeNone},
    {"rx_enable", funcNone, typeUChar, typeNone, typeNone},
    {"xit_offset", funcNone, typeUChar, typeNone, typeNone},
    {"rx_channel_enable", funcNone, typeUChar, typeNone, typeNone},
    {"rx_filter_band", funcNone, typeUChar, typeNone, typeNone},
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
    {"cw_msg_stop", funcNone, typeUChar, typeNone, typeNone}, // TCI 2.0 command (inbound only)
    {"spot", funcNone, typeUChar, typeUChar, typeNone},
    {"spot_delete", funcNone, typeUChar, typeUChar, typeNone},
    {"iq_samplerate", funcNone, typeUChar, typeNone, typeNone},
    {"audio_samplerate", funcNone, typeUChar, typeNone, typeNone},
    {"iq_start", funcNone, typeUChar, typeNone, typeNone},
    {"iq_stop", funcNone, typeUChar, typeNone, typeNone},
    {"audio_start", funcNone, typeUChar, typeNone, typeNone},
    {"audio_stop", funcNone, typeUChar, typeNone, typeNone},
    {"tx_audio_start", funcNone, typeNone, typeNone,
     typeNone}, // TCI 2.0 command (inbound only)
    {"tx_audio_stop", funcNone, typeNone, typeNone,
     typeNone}, // TCI 2.0 command (inbound only)
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
    {"rx_sensors", funcSMeter, typeUChar, typeDouble, typeNone}, // RX_SENSORS:trx,dBm
    {0x0, funcNone, typeNone, typeNone, typeNone},
};

// Optional strict mode for client testing: drop TX audio unless the client
// keyed with "trx:<n>,true,tci;" as TCI 2.1 requires.
static const bool kStrictTciSource =
    qEnvironmentVariableIsSet("WFVIEW_TCI_STRICT_SOURCE");

// Map a rig's raw level (its min..max) to the TCI scale (lo..hi) and back.
static int rawToScale(rigCapabilities *caps, funcs f, int raw, int lo, int hi) {
    if (!caps || !caps->commands.contains(f)) return lo;
    auto c = caps->commands.find(f);
    const int span = int(c->maxVal) - int(c->minVal);
    if (span <= 0) return lo;
    return qRound(lo + double(raw - int(c->minVal)) * (hi - lo) / span);
}
static int scaleToRaw(rigCapabilities *caps, funcs f, int v, int lo, int hi) {
    if (!caps || !caps->commands.contains(f)) return 0;
    auto c = caps->commands.find(f);
    v = qBound(lo, v, hi);
    return qRound(int(c->minVal) + double(v - lo) * (int(c->maxVal) - int(c->minVal)) / (hi - lo));
}

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

    // Mark this client as TCI 2.x stream-aware. Legacy TCI 1.x clients (such as
    // WSJT-X / WS Digital Mode Suite) never send audio_stream_* configuration
    // commands and leave Stream::reserv[0] uninitialized in binary headers.
    c.streamConfigured = true;

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

    // TCI v2.0 / v2.1 Protocol handshake
    pSocket->sendTextMessage(QString("protocol:WFVIEW,2.1;"));
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

    // TCI 2.0 / 2.1 stream defaults and TX capability announcement.
    // Hamlib / WSJT-X clients check tx_enable on connect before allowing TX.
    pSocket->sendTextMessage(QString("audio_stream_sample_type:float32;"));
    pSocket->sendTextMessage(QString("audio_stream_channels:2;"));
    pSocket->sendTextMessage(QString("audio_stream_samples:2048;"));
    pSocket->sendTextMessage(QString("iq_samplerate:48000;"));
    pSocket->sendTextMessage(QString("audio_samplerate:48000;"));
    pSocket->sendTextMessage(QString("tx_enable:0,%0;")
                                 .arg(rigCaps->hasTransmit ? "true" : "false"));

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

    pSocket->sendTextMessage(QString("rx_enable:0,true;"));
    pSocket->sendTextMessage(QString("trx:0,false;"));
    pSocket->sendTextMessage(QString("start;"));
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

    // One WebSocket frame may carry several ';'-terminated commands. Reserved
    // characters can never appear inside an argument, so splitting is safe.
    {
        const QStringList cmds = message.split(';', Qt::SkipEmptyParts);
        if (cmds.size() > 1) {
            for (const QString &c : cmds)
                processIncomingTextMessage(c.trimmed() + ";"); // sender() is unchanged
            return;
        }
    }

    it.value().connected = true;
    qDebug() << "TCI Text Message received:" << message;

    QString cmd = message.section(':', 0, 0).toLower().trimmed();
    if (cmd.endsWith(';'))
        cmd.chop(1); // zero-argument commands ("start;", "volume;") keep it
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

    if (name == "rx_sensors") // server -> client only; ignore from clients
        return;
    if (name == "rx_sensors_enable") {
        if (arg.isEmpty())
            return;
        const bool on = arg.at(0).compare("true", Qt::CaseInsensitive) == 0 ||
                        arg.at(0) == "1";
        bool ok = false;
        int ms = arg.size() > 1 ? arg.at(1).toInt(&ok) : 100;
        if (arg.size() > 1 && !ok)
            ms = 100;
        pClient->setProperty("rxSensors", on);
        pClient->setProperty("rxSensMs", qBound(30, ms, 1000));
        pClient->setProperty("rxSensLvl", QVariant());
        pClient->setProperty("rxSensT", qint64(0));
        return;
    }

    // ---- Backward compatibility (inbound only; the server only ever sends
    // canonical TCI 2.1 commands). Rewrite old forms to their 2.1 equivalent.
    if (cmd == "transceiver") {                 // 2.0 name for TRX
        cmd = "trx";
    } else if (cmd == "tx_power" && arg.size() == 1) {   // power without a trx number
        cmd = "drive";
        arg.prepend("0");
        rest = arg.join(',');
    } else if (cmd == "drive" && arg.size() == 1) {
        // TCI 1.x "drive:<power>;" vs TCI 2.1 read "drive:<trx>;". A value that
        // cannot be a transceiver number is the old set form. A value that can
        // (0 on a single-receiver rig) is treated as a 2.1 read, so a legacy
        // client can never silently set the power to zero.
        bool ok = false;
        const uint n = arg.at(0).toUInt(&ok);
        if (ok && n >= uint(rigCaps->numReceiver)) {
            arg.prepend("0");
            rest = arg.join(',');
        }
    }

    uchar rx = 0;
    if (arg.size() > 0) {
        rx = arg.at(0).toUInt();
    }
    uchar vfo = 0;
    if (arg.size() > 1) {
        vfo = arg.at(1).toUInt();
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

            if (tc.func != funcNone && tc.arg1 == typeUChar && numArgs > 1) {
                // arg1 is a receiver/transceiver number: must exist and be in range
                bool ok = false;
                const uint r = arg.isEmpty() ? 0u : arg.at(0).toUInt(&ok);
                if (!ok || r >= uint(rigCaps->numReceiver))
                    return;
            }

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
                    val = QVariant::fromValue(uchar(arg.at(0).toInt(NULL)));
                }
                if (arg.count() == 1 && tc.arg1 == typeUShort) {
                    val = QVariant::fromValue(
                        ushort(arg.at(0).toInt(NULL) * 2.55));
                }
                if (arg.count() == 2 && tc.arg2 == typeUChar) {
                    val = QVariant::fromValue(uchar(arg.at(1).toInt(NULL)));
                }
                if (arg.count() == 2 && tc.arg2 == typeUShort) {
                    val = QVariant::fromValue(
                        ushort(arg.at(1).toInt(NULL) * 2.55));
                } else if (arg.count() == 3 && tc.arg3 == typeFreq) {
                    val = QVariant::fromValue<freqt>(freqt(
                        arg.at(2).toUInt(), arg.at(2).toUInt() / (double)1E6,
                        selVFO_t::activeVFO));
                    func = t.freqFunc;
                } else if (tc.arg2 == typeMode) {
                    val = QVariant::fromValue<modeInfo>(rigMode(arg.at(1)));
                    func = t.modeFunc;
                } else if (tc.arg2 == typeBinary) {
                    val = QVariant::fromValue(
                        arg.at(1).compare("true", Qt::CaseInsensitive) == 0 ||
                        arg.at(1) == "1");
                }

                if (func == funcAfGain && numArgs == 1) { // VOLUME:dB (-60..0)
                    bool ok = false;
                    const int db = arg.at(0).toInt(&ok);
                    if (!ok)
                        return;
                    val = QVariant::fromValue(
                        uchar(scaleToRaw(rigCaps, funcAfGain, db, -60, 0)));
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
                    it.value().txPending.clear();
                    reply = "tx_audio_stop;";
                }
                updateAudioState();
                pClient->sendTextMessage(reply);
                return;
            }

            // Clear buffered legacy TX audio when unkeying ("trx:0,false;"),
            // and answer TX_ENABLE read queries per the TCI 2.1 specification.
            if (cmd == "trx" && set && arg.size() >= 2) {
                reply = QString("trx:%0,%1;").arg(rx).arg(arg.at(1).toLower());
                const bool keyed =
                    arg.at(1).compare("true", Qt::CaseInsensitive) == 0 ||
                    arg.at(1) == "1";
                it.value().txPending.clear();

                // Remember whether TCI audio was requested as the TX source
                // (TCI 2.1 TRX arg3). Used by the source check in the binary
                // handler.
                pClient->setProperty("txKeyed", keyed);
                pClient->setProperty(
                    "txSrcTci",
                    keyed && arg.size() > 2 &&
                        arg.at(2).compare("tci", Qt::CaseInsensitive) == 0);
                pClient->setProperty("txSrcWarned", false);

                // Replace any running TX_CHRONO clock for this client.
                if (QTimer *old = pClient->findChild<QTimer *>(
                        "tciChrono", Qt::FindDirectChildrenOnly)) {
                    old->stop();
                    old->deleteLater();
                }
                if (keyed) {
                    QTimer *t = new QTimer(pClient);
                    t->setObjectName("tciChrono");
                    t->setTimerType(Qt::PreciseTimer);
                    t->setInterval(4);
                    QElapsedTimer clock;
                    clock.start();
                    qint64 sent = 0;
                    QPointer<QWebSocket> sock(pClient);
                    connect(t, &QTimer::timeout, this,
                            [this, sock, clock, sent]() mutable {
                        if (!sock) return;
                        auto cit = clients.find(sock.data());
                        if (cit == clients.end()) return;
                        connStatus &c = cit.value();
                        const quint32 ch = (c.channels == 1) ? 1u : 2u;
                        const quint32 total = quint32(c.samples);   // scalar samples/packet
                        const qint64 periodNs =
                            qint64(total / ch) * 1000000000LL / 48000;
                        const qint64 due = clock.nsecsElapsed() / periodNs;
                        if (due - sent > 8) sent = due - 1;         // resync after a stall
                        while (sent < due) {
                            dataStream *p = reinterpret_cast<dataStream *>(txChrono.data());
                            p->receiver = 0;
                            p->sampleRate = 48000;
                            // Legacy (1.x) clients never negotiate a format: always float32.
                            p->format = quint32(c.streamConfigured ? quint32(c.sampleType)
                                                                   : quint32(IqFloat32));
                            p->codec = 0u;
                            p->crc = 0u;
                            p->type = TxChrono;
                            p->length = total;
                            p->reserv[0] = ch;
                            p->reserv[1] = c.chronoCount++;
                            sock->sendBinaryMessage(txChrono);
                            ++sent;
                        }
                    });
                    t->start();
                }
            } else if (cmd == "tx_enable") {
                reply = QString("tx_enable:%0,%1;")
                            .arg(rx)
                            .arg(rigCaps->hasTransmit ? "true" : "false");
                pClient->sendTextMessage(reply);
                return;
            }

            if (tc.func != funcNone && !set) {
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
                    reply += QString(":%0").arg(arg.at(0));
                }

                if (tc.func == funcAfGain && numArgs == 1) {
                    reply += QString(":%0").arg(
                        rawToScale(rigCaps, funcAfGain, v.toInt(), -60, 0));
                } else if (numArgs == 1 && tc.arg1 == typeUChar) {
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

    auto it = clients.find(pClient);
    if (it == clients.end())
        return;

    const dataStream *pStream =
        reinterpret_cast<const dataStream *>(message.constData());
    if (pStream->type != TxAudioStream || pStream->length == 0)
        return;

    // TCI 2.1 (TRX note): audio from the TCI stream is only used when TRX was
    // sent with source "tci"; otherwise the signal comes from the microphone.
    // By default this server is lenient and forwards the audio anyway, but
    // warns once per key-down so a client that forgets the argument is visible.
    // Set WFVIEW_TCI_STRICT_SOURCE in the environment to drop such audio.
    {
        const bool keyed = pClient->property("txKeyed").toBool();
        const bool viaTci = pClient->property("txSrcTci").toBool();
        if (!keyed || !viaTci) {
            if (keyed && !pClient->property("txSrcWarned").toBool()) {
                pClient->setProperty("txSrcWarned", true);
                qWarning(logTCIServer())
                    << "TCI client sent TX audio, but TRX was not requested with "
                       "source 'tci' (TCI 2.1: signal is then taken from the "
                       "microphone)"
                    << (kStrictTciSource ? "- audio dropped" : "- forwarding anyway");
            }
            if (kStrictTciSource)
                return;
        }
    }

    QByteArray tempData;

    if (pStream->format == IqFloat32) {
        const qsizetype available = (message.size() - qsizetype(iqHeaderSize)) /
                                    qsizetype(sizeof(float));
        const float *inFloats =
            reinterpret_cast<const float *>(message.constData() + iqHeaderSize);

        if (it.value().streamConfigured) {
            // --- Strict TCI 2.1 Spec Path
            // --- pStream->reserv[0] reliably carries channel count (1 or 2),
            // and pStream->length carries total scalar floats in the packet.
            const qsizetype count = qMin<qsizetype>(pStream->length, available);
            if (pStream->reserv[0] == 2) {
                const qsizetype frames = count / 2;
                tempData.resize(frames * qsizetype(sizeof(float)));
                float *outFloats = reinterpret_cast<float *>(tempData.data());
                for (qsizetype i = 0; i < frames; ++i) {
                    outFloats[i] = inFloats[2 * i];
                }
            } else {
                tempData.resize(count * qsizetype(sizeof(float)));
                memcpy(tempData.data(), inFloats, tempData.size());
            }
        } else {
            // --- Legacy TCI 1.x Compatibility Path (WSJT-X / WS Digital Mode
            // Suite) --- Upstream WSJT-X never merged Chris Esser's TCI fix PR:
            // 1. It never sends audio_stream_* configuration commands.
            // 2. It leaves Stream::reserv (channels) uninitialized (random stack garbage).
            // 3. It sends a fixed 16448-byte buffer (64-byte header + 4096 floats) with
            //   - pStream->length = 2048 (1024 interleaved stereo frames in
            //   - inFloats[0..2047]), while leaving inFloats[2048..4095]
            //   - uninitialized (~1e38 float garbage).
            // Never read beyond pStream->length (2048 floats), or uninitialized
            // tail noise will modulate across the entire 3 kHz SSB passband.
            //
            // WSJT-X developers either never read the spec, don't understand the
            // spec, or don't know what they're doing.
            const qsizetype count = qMin(qsizetype(pStream->length), available);
            const qsizetype frames = count / 2;
            // Allocate space matching downstream float requirements
            tempData.resize(frames * qsizetype(sizeof(float)));
            float *outFloats = reinterpret_cast<float *>(tempData.data());
            for (qsizetype i = 0; i < frames; ++i) {
                outFloats[i] = 0.5f * (inFloats[2 * i] + inFloats[2 * i + 1]);
            }
        }
    } else if (pStream->format == IqInt16) {
        const qsizetype available = (message.size() - qsizetype(iqHeaderSize)) /
                                    qsizetype(sizeof(int16_t));
        const qsizetype count = qMin(qsizetype(pStream->length), available);
        const int16_t *inInts = reinterpret_cast<const int16_t *>(
            message.constData() + iqHeaderSize);
        if (pStream->reserv[0] == 2) {
            const qsizetype frames = count / 2;
            tempData.resize(frames * qsizetype(sizeof(float)));
            float *outFloats = reinterpret_cast<float *>(tempData.data());
            for (qsizetype i = 0; i < frames; ++i) {
                outFloats[i] = float(inInts[2 * i]) / 32768.0f;
            }
        } else {
            tempData.resize(count * qsizetype(sizeof(float)));
            float *outFloats = reinterpret_cast<float *>(tempData.data());
            for (qsizetype i = 0; i < count; ++i) {
                outFloats[i] = float(inInts[i]) / 32768.0f;
            }
        }
    } else if (pStream->format == IqInt24 || pStream->format == IqInt32) {
        const int bps = (pStream->format == IqInt24) ? 3 : 4;
        const qsizetype available = (message.size() - qsizetype(iqHeaderSize)) / bps;
        const qsizetype count = qMin(qsizetype(pStream->length), available);
        const uint8_t *in =
            reinterpret_cast<const uint8_t *>(message.constData()) + iqHeaderSize;
        auto sampleAt = [&](qsizetype n) -> float {
            const uint8_t *q = in + n * bps;
            if (bps == 3) {
                int32_t v = int32_t(q[0]) | (int32_t(q[1]) << 8) | (int32_t(q[2]) << 16);
                if (v & 0x800000)
                    v |= int32_t(0xFF000000u); // sign-extend 24 -> 32 bit
                return float(v) / 8388608.0f;
            }
            int32_t v;
            std::memcpy(&v, q, 4);
            return float(double(v) / 2147483648.0);
        };
        const bool stereo = (pStream->reserv[0] == 2);
        const qsizetype outN = stereo ? count / 2 : count;
        tempData.resize(outN * qsizetype(sizeof(float)));
        float *outFloats = reinterpret_cast<float *>(tempData.data());
        for (qsizetype i = 0; i < outN; ++i)
            outFloats[i] = sampleAt(stereo ? 2 * i : i);
    } else {
        // If an unhandled format type arrives, ignore per protocol spec
        qWarning(logTCIServer()) << "TCI TX audio dropped: unsupported format"
                                 << pStream->format;
        return;
    }
    // Buffer into 2048-float (8192-byte) mono blocks for audioHandlerTciOutput.
    // For TCI 2.1 clients sending 2048-float blocks, this passes straight
    // through with zero added latency; for 1024-frame chunks, it pairs two
    // chunks cleanly.
    it.value().txPending.append(tempData);
    const qsizetype txBlockBytes = 2048 * qsizetype(sizeof(float));
    while (it.value().txPending.size() >= txBlockBytes) {
        emit sendTCIAudio(it.value().txPending.left(txBlockBytes));
        it.value().txPending.remove(0, txBlockBytes);
    }
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

    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (!it.value().connected || !it.value().rxaudio)
            continue;

        connStatus &c = it.value();
        c.rxPending.append(audio.data);

        // TCI 2.1 standard framing: c.samples is total scalar samples per
        // packet (e.g., 2048 scalar samples / 2 channels = 1024 stereo frames
        // = 21.33 ms). Legacy TCI 1.x clients (WSJT-X) default to 2 channels
        // and expect this exact 48 kHz stereo RxAudioStream cadence for
        // decoding.
        const quint32 ch = (c.channels == 1) ? 1u : 2u;
        const qsizetype framesPerPacket = c.samples / ch;
        const qsizetype bytesNeeded =
            framesPerPacket * qsizetype(sizeof(float));

        while (c.rxPending.size() >= bytesNeeded) {
            const float *inSamples =
                reinterpret_cast<const float *>(c.rxPending.constData());
            const quint32 totalSamples =
                static_cast<quint32>(framesPerPacket * ch);
            const qsizetype payloadBytes =
                totalSamples * qsizetype(sizeof(float));

            // TX_CHRONO is generated by a per-client timer while keyed (see "trx").

            // Send RxAudioStream packet
            rxAudioData.resize(iqHeaderSize + payloadBytes);
            std::memset(rxAudioData.data(), 0, iqHeaderSize);
            dataStream *pStream =
                reinterpret_cast<dataStream *>(rxAudioData.data());

            pStream->receiver = 0;
            pStream->sampleRate = 48000;
            pStream->format = IqFloat32;
            pStream->codec = 0u;
            pStream->type = RxAudioStream;
            pStream->crc = 0u;
            pStream->length = totalSamples;
            pStream->reserv[0] = ch;
            pStream->reserv[1] = c.rxCount++;

            if (ch == 2) {
                float *out = pStream->data;
                for (qsizetype i = 0; i < framesPerPacket; ++i) {
                    out[2 * i] = inSamples[i];
                    out[2 * i + 1] = inSamples[i];
                }
            } else {
                std::memcpy(pStream->data, inSamples, payloadBytes);
            }

            if (c.streamConfigured && c.sampleType != IqFloat32) {
                // Convert the float payload in place; the write position never
                // passes the read position because every int sample is <= 4 bytes.
                const int bps = (c.sampleType == IqInt16) ? 2
                              : (c.sampleType == IqInt24) ? 3 : 4;
                uint8_t *w = reinterpret_cast<uint8_t *>(rxAudioData.data()) + iqHeaderSize;
                const float *f = reinterpret_cast<const float *>(w);
                for (quint32 n = 0; n < totalSamples; ++n) {
                    const float x = qBound(-1.0f, f[n], 1.0f);
                    if (bps == 2) {
                        const int16_t v = int16_t(qRound(x * 32767.0f));
                        std::memcpy(w + n * 2, &v, 2);
                    } else if (bps == 3) {
                        const int32_t v = int32_t(qRound(x * 8388607.0f));
                        w[n * 3] = uint8_t(v & 0xFF);
                        w[n * 3 + 1] = uint8_t((v >> 8) & 0xFF);
                        w[n * 3 + 2] = uint8_t((v >> 16) & 0xFF);
                    } else {
                        const int32_t v = int32_t(double(x) * 2147483647.0);
                        std::memcpy(w + n * 4, &v, 4);
                    }
                }
                pStream = reinterpret_cast<dataStream *>(rxAudioData.data());
                pStream->format = c.sampleType;
                rxAudioData.resize(qsizetype(iqHeaderSize) + qsizetype(totalSamples) * bps);
            }

            it.key()->sendBinaryMessage(rxAudioData);
            c.rxPending.remove(0, bytesNeeded);
        }
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
                    reply.clear();
                    tciCommandStruct tc = tci_commands[i];
                    uchar numArgs = 0;
                    if (tc.arg1 != typeNone)
                        numArgs++;
                    if (tc.arg2 != typeNone)
                        numArgs++;
                    if (tc.arg3 != typeNone)
                        numArgs++;

                    if (tc.func == funcSMeter) {
                        // RX_SENSORS: only for clients that enabled it, rate
                        // limited, and only when the level changes.
                        QWebSocket *sock = it.key();
                        if (!sock->property("rxSensors").toBool())
                            continue;
                        const int lvl = qRound(item.value.value<double>() - dBmConversion);
                        const qint64 now = QDateTime::currentMSecsSinceEpoch();
                        const QVariant last = sock->property("rxSensLvl");
                        if ((last.isValid() && last.toInt() == lvl) ||
                            now - sock->property("rxSensT").toLongLong() <
                                sock->property("rxSensMs").toInt())
                            continue;
                        sock->setProperty("rxSensLvl", lvl);
                        sock->setProperty("rxSensT", now);
                    }

                    if (tc.func != funcNone) {
                        if (tc.arg1 == typeUChar && numArgs > 1)
                            reply = QString("%0:%1").arg(tc.str).arg(
                                QString::number(item.receiver));
                        else if (tc.func == funcAfGain && numArgs == 1)
                            reply = QString("%0:%1").arg(tc.str).arg(
                                rawToScale(rigCaps, funcAfGain,
                                           item.value.toInt(), -60, 0));
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
                        } else if (numArgs == 2 && tc.arg2 == typeDouble) {
                            reply += QString(",%0").arg(
                                qRound(item.value.value<double>() - dBmConversion));
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

