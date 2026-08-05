#ifndef ACTION_DISPATCHER_H
#define ACTION_DISPATCHER_H

class Dispatcher {
public:
        // Received requests
        static constexpr char const* PATH_START_RECORDING = "/recorder/start";
        // Response
        static constexpr char const* LOG_START_RECORDING = "Recording started";

        // Sent requests
        static constexpr char const* PATH_RECORDING_DONE = "/recorder/done";
        static constexpr char const* PATH_SEND_FILE = "/file/send";
        // Request body text (might not need)
        static constexpr char const* LOG_RECORDING_DONE = "Recording finished";
        static constexpr char const* LOG_SEND_FILE = "File sent";

        static constexpr char const* LOG_NO_ACTION = "No action selected";
        static constexpr char const* LOG_UNKNOWN_ACTION = "Unsupported action";

        enum class Action {
                NONE,
                START_RECORDING,
                NOTIFY_STOPPED,
                SEND_FILE,
                UNKNOWN
        };

        void update();

        bool performImmediateAction(const char* path);

        bool setPendingAction(const char* path);
        bool performPendingAction();
        const char* pendingAction();
private:
        Action pendingAction_ = Action::NONE;

        enum class Status { IDLE, PENDING, DISPATCHING };

        Status status = Status::IDLE;

        bool selectAction(const char* path);
        bool performAction();
};

#endif