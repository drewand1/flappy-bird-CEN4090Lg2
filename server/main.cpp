#include <httplib.h>

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {
	constexpr std::uint16_t DEFAULT_PORT = 8080;
	constexpr const char* DEFAULT_SCORE_FILE = "scores.txt";
	constexpr std::size_t BOARD_SIZE = 5;
	constexpr std::size_t MAX_NAME_LENGTH = 12; // keep in sync with the gameconfig.h
	constexpr std::size_t MAX_SCORE_DIGITS = 9;

	bool isValidName(std::string_view name) {
		const auto isAlnum = [](char c) {
			return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
		};
		return !name.empty() && name.size() <= MAX_NAME_LENGTH && std::ranges::all_of(name, isAlnum);
	}

	template <typename T>
	std::optional<T> parseNumber(std::string_view text, std::size_t maxDigits) {
		T value{};
		const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
		if (text.empty() || text.size() > maxDigits || ec != std::errc{} || end != text.data() + text.size())
			return std::nullopt;
		return value;
	}

	class Leaderboard {
	public:
		explicit Leaderboard(std::filesystem::path file) : mFile(std::move(file)) {
			std::ifstream in(mFile);
			std::string name, score;
			while (in >> name >> score) {
				const auto value = parseNumber<std::uint32_t>(score, MAX_SCORE_DIGITS);
				if (isValidName(name) && value)
					insert(name, *value);
			}
		}

		// returns true if the board changed.
		bool submit(const std::string& name, std::uint32_t score) {
			const std::lock_guard lock(mMutex);
			if (!insert(name, score))
				return false;

			auto tmp = mFile;
			tmp += ".tmp";
			std::ofstream(tmp) << serializeLocked();
			std::error_code ec;
			std::filesystem::rename(tmp, mFile, ec);
			if (ec)
				std::cerr << "[scoreboard] failed saving " << mFile << ": " << ec.message() << "\n";
			return true;
		}

		std::string serialize() const {
			const std::lock_guard lock(mMutex);
			return serializeLocked();
		}

	private:
		struct Entry {
			std::string name;
			std::uint32_t score;
			bool operator==(const Entry&) const = default;
		};

		bool insert(const std::string& name, std::uint32_t score) {
			const std::vector<Entry> before = mEntries;
			const auto existing = std::ranges::find(mEntries, name, &Entry::name);
			if (existing != mEntries.end() && existing->score >= score)
				return false;
			if (existing != mEntries.end())
				existing->score = score;
			else
				mEntries.push_back({name, score});

			std::ranges::stable_sort(mEntries, std::ranges::greater{}, &Entry::score);
			if (mEntries.size() > BOARD_SIZE)
				mEntries.resize(BOARD_SIZE);
			return mEntries != before;
		}

		std::string serializeLocked() const {
			std::string out;
			for (const Entry& e : mEntries)
				out += e.name + ' ' + std::to_string(e.score) + '\n';
			return out;
		}

		std::filesystem::path mFile;
		std::vector<Entry> mEntries;
		mutable std::mutex mMutex;
	};
}

int main(int argc, char** argv) {
	const std::optional<std::uint16_t> port = argc > 1 ? parseNumber<std::uint16_t>(argv[1], 5) : DEFAULT_PORT;
	if (argc > 3 || !port || *port == 0) {
		std::cerr << "usage: " << argv[0] << " [port (default " << DEFAULT_PORT << ")] [score-file (default " << DEFAULT_SCORE_FILE << ")]\n";
		return 2;
	}
	const std::string scoreFile = argc > 2 ? argv[2] : DEFAULT_SCORE_FILE;
	Leaderboard board(scoreFile);

	httplib::Server server;
	server.set_payload_max_length(256);
	server.set_read_timeout(5);
	server.set_write_timeout(5);
	server.set_keep_alive_max_count(1); // one request per connection

	server.Get("/scores", [&](const httplib::Request&, httplib::Response& res) {
		res.set_content(board.serialize(), "text/plain; charset=utf-8");
	});

	server.Post("/scores", [&](const httplib::Request& req, httplib::Response& res) {
		const std::string name = req.get_param_value("name");
		const auto score = parseNumber<std::uint32_t>(req.get_param_value("score"), MAX_SCORE_DIGITS);
		if (req.params.size() != 2 || !isValidName(name) || !score) {
			res.status = httplib::StatusCode::BadRequest_400;
			res.set_content("expected name=<1-12 letters/digits>&score=<number>\n", "text/plain; charset=utf-8");
			return;
		}
		if (board.submit(name, *score))
			std::cout << "[scoreboard] new top score: " << name << " " << *score << std::endl;
		res.status = httplib::StatusCode::NoContent_204;
	});

	std::cout << "[scoreboard] listening on 0.0.0.0:" << *port << ", scores in " << scoreFile << std::endl;
	if (!server.listen("0.0.0.0", *port)) {
		std::cerr << "[scoreboard] could not listen on port " << *port << "\n";
		return 1;
	}
}
