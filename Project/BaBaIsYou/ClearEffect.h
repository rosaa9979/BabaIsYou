enum class ClearPhase {
    Idle,
    CloseOut,
    Done
};

struct ClearEffect {
    ClearPhase phase = ClearPhase::Idle;
    float timer = 0.0f;
    float radiusProgress = 0.0f;

    void Start() {
        phase = ClearPhase::CloseOut;
        timer = 0.0f;
        radiusProgress = 0.0f;  // 중심에서부터 시작
    }

    void Update(float deltaTime) {
        if (phase == ClearPhase::CloseOut) {
            timer += deltaTime;

            if (timer < 2.0f)
                radiusProgress = timer / 2.0f;
            else {
                radiusProgress = 1.0f;
                phase = ClearPhase::Done;
            }
        }
    }

    bool IsActive() const {
        return phase != ClearPhase::Done;
    }

    bool IsFinished() const {
        return phase == ClearPhase::Done;
    }

    bool IsRendering() const {
        return phase == ClearPhase::CloseOut || phase == ClearPhase::Done;
    }

    ClearPhase GetPhase() const {
        return phase;
    }
};