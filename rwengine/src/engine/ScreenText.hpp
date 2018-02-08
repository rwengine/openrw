#ifndef _RWENGINE_SCREENTEXT_HPP_
#define _RWENGINE_SCREENTEXT_HPP_
#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <data/GameTexts.hpp>

enum class ScreenTextType {
    /// Big text will be rendered according to the proscribed style.
    /// Adding a 2nd big text will cause the first to terminate.
    Big = 0,
    /// See Big, will wait for any Big text to finish.
    BigLowPriority = 1,
    /// Will be cleared by the clear help opcode
    Help = 2,
    /// Automatically cleared after each tick, for generic text.
    Immediate = 3,
    /// High priority cutscene text
    HighPriority = 4,
    ///
    _Count = 5
};
constexpr unsigned int ScreenTypeTextCount =
    static_cast<unsigned int>(ScreenTextType::_Count);

/**
 * @brief The ScreenTextEntry struct
 *
 * Text string and fading information.
 */
struct ScreenTextEntry {
    /// After processing numbers
    GameString text;
    /// in the virtual 640x480 screen space
    glm::vec2 position{};
    /// Font number
    int font;
    /// Font size
    int size;
    /// Background colour (or, if a == 0, shadow offset)
    glm::u8vec4 colourBG{};
    /// Foreground colour
    glm::u8vec3 colourFG{};
    /// Alignment (left = 0, center = 1, right = 2)
    unsigned char alignment;
    /// Onscreen duration
    int durationMS;
    /// The amount of time onscreen so far
    int displayedMS;
    /// Wrap width
    int wrapX;
    /// ID used to reference the text.
    GameStringKey id;

    static ScreenTextEntry makeBig(const GameStringKey& id,
                                   const GameString& str, int style,
                                   int durationMS);

    static ScreenTextEntry makeHighPriority(const GameStringKey& id,
                                            const GameString& str,
                                            int durationMS);

    static ScreenTextEntry makeHelp(const GameStringKey& id,
                                    const GameString& str);
};

/**
 * @brief The ScreenText class
 *
 * Logic for on-screen game text rendering
 *
 * There are 4 text pool types:
 * - Big Text
 * - (low priority) Big Text
 * - Help Text
 * - Immediate Text
 *
 * Only one Big Text can be drawn at a time, adding a second will
 * cause the first to end prematurely. Low priority big text will
 * only display if there is no regular big text.
 *
 * Help text.
 *
 * Immediate text is only rendered once, the text is removed at the
 * start of each tick and must be re-added to keep it on screen.
 */
class ScreenText {
    using EntryList = std::vector<ScreenTextEntry>;
    using EntryQueues = std::array<EntryList, ScreenTypeTextCount>;

public:
    ///
    /// \brief tick Apply display and fading rules to the text
    /// \param dt
    ///
    void tick(float dt);

    template <ScreenTextType Q, class... Args>
    void addText(Args&&... args) {
        static_assert(static_cast<size_t>(Q) < ScreenTypeTextCount,
                      "Queue out of range");
        m_textQueues[static_cast<size_t>(Q)].emplace_back(
            std::forward<Args...>(args...));
    }

    template <ScreenTextType Q>
    const EntryList& getText() const {
        static_assert(static_cast<size_t>(Q) < ScreenTypeTextCount,
                      "Queue out of range");
        return m_textQueues[static_cast<size_t>(Q)];
    }

    template <ScreenTextType Q>
    void clear() {
        static_assert(static_cast<size_t>(Q) < ScreenTypeTextCount,
                      "Queue out of range");
        m_textQueues[static_cast<size_t>(Q)].clear();
    }

    template <ScreenTextType Q>
    void remove(const std::string& id) {
        static_assert(static_cast<size_t>(Q) < ScreenTypeTextCount,
                      "Queue out of range");
        auto& list = m_textQueues[static_cast<size_t>(Q)];
        list.erase(std::remove_if(
                       list.begin(), list.end(),
                       [&id](const ScreenTextEntry& e) { return e.id == id; }),
                   list.end());
    }

    const EntryQueues& getAllText() const {
        return m_textQueues;
    }

    template <class... Args>
    static GameString format(GameString format, Args&&... args) {
        static auto kReplacementMarker = GameStringUtil::fromString("~1~");
        const std::array<GameString, sizeof...(args)> vals = {{args...}};
        size_t x = 0, val = 0;
        // We're only looking for numerical replacement markers
        while ((x = format.find(kReplacementMarker)) != GameString::npos &&
               val < vals.size()) {
            format = format.substr(0, x) + vals[val++] + format.substr(x + 3);
        }
        return format;
    }

private:
    EntryQueues m_textQueues;
};

#endif
