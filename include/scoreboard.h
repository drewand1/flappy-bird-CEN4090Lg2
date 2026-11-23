#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

struct ScoreEntry {
	std::string name;
	unsigned int score = 0;
};

// communicates with central scoreboard over plain http
// requests block so need to be made outside main thread
class ScoreboardClient {
public:
	ScoreboardClient();
	ScoreboardClient(std::string host, std::uint16_t port);

	[[nodiscard]] std::expected<void, std::string> submitScore(std::string_view name, unsigned int score) const;
	[[nodiscard]] std::expected<std::vector<ScoreEntry>, std::string> topScores() const;

	[[nodiscard]] static bool isValidName(std::string_view name) noexcept;

private:
	std::string mHost;
	std::uint16_t mPort;
};

#endif
