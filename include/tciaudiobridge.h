#ifndef TCIAUDIOBRIDGE_H
#define TCIAUDIOBRIDGE_H

#include <QList>
#include <QObject>
#include <QThread>

#include "audiohandler.h"

/**
 * @brief Carries audio between a USB-connected radio's sound card and the TCI
 * server.
 *
 * wfview only creates audio handlers for the radio when a LAN client asks for a
 * stream, so a TCI client on its own gets no audio. This class opens the
 * radio's USB codec itself and feeds it to, and takes TX audio from, the TCI
 * server.
 */
class tciAudioBridge : public QObject {
    Q_OBJECT

  public:
    explicit tciAudioBridge(QObject *parent = nullptr);
    ~tciAudioBridge() override;

    /**
     * @brief Open the radio's audio device and connect it to the TCI server.
     * @param rxDevice Setup for capturing the radio's receive audio (server RX
     * input).
     * @param txDevice Setup for playing transmit audio to the radio (server TX
     * output).
     * @param tci The tciServer instance, passed as void* like audioSetup::tci.
     */
    void start(const audioSetup &rxDevice, const audioSetup &txDevice,
               void *tci);

    /** @brief Close all handlers and stop their threads. Safe to call when not
     * running. */
    void stop();

    bool isRunning() const { return running; }

  private:
    audioHandlerBase *makeInput(audioType type);
    audioHandlerBase *makeOutput(audioType type);
    void place(audioHandlerBase *handler, const QString &name);
    void initHandler(audioHandlerBase *handler, const audioSetup &setup);

    audioHandlerBase *rxCapture = nullptr; ///< USB codec -> radio-format audio
    audioHandlerBase *rxToTci =
        nullptr; ///< radio-format -> float32 -> TCI server
    audioHandlerBase *txFromTci =
        nullptr; ///< TCI server float32 -> radio-format
    audioHandlerBase *txPlayback = nullptr; ///< radio-format -> USB codec
    QList<QThread *> threads;
    bool running = false;
};

#endif // TCIAUDIOBRIDGE_H
