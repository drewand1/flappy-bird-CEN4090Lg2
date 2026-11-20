#include "scoreboard.h"
#include "gameconfig.h"

#include <httplib.h>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <utility>

namespace {
	constexpr auto REQUEST_TIMEOUT = std::chrono::seconds(4);
	constexpr std::size_t MAX_RESPONSE_BYTES = 8192;

	httplib::Client makeClient(const std::string& host, std::uint16_t port) {
		httplib::Client client(host, port);
		client.set_connection_timeout(REQUEST_TIMEOUT);
		client.set_read_timeout(REQUEST_TIMEOUT);
		client.set_write_timeout(REQUEST_TIMEOUT);
		client.set_max_timeout(REQUEST_TIMEOUT);
		client.set_payload_max_length(MAX_RESPONSE_BYTES);
		return client;
	}

	// ASCII digits only
	std::optional<unsigned int> parseUnsigned(std::string_view text) noexcept {
		unsigned int value = 0;
		const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
		if (text.empty() || ec != std::errc{} || end != text.data() + text.size())
			return std::nullopt;
		return value;
	}

	// hostnames only
	bool isValidHost(std::string_view host) noexcept {
		const auto ok = [](char c) {
			return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '.' || c == '-';
		};
		return !host.empty() && host.size() <= 253 && std::ranges::all_of(host, ok);
	}
}

ScoreboardClient::ScoreboardClient()
	: mHost(SCOREBOARD_DEFAULT_HOST), mPort(SCOREBOARD_DEFAULT_PORT) {
	const char* env = std::getenv("FLAPPY_SCOREBOARD");
	if (!env || !*env)
		return;

	const std::string_view endpoint(env);
	const auto colon = endpoint.rfind(':');
	const std::string_view host = endpoint.substr(0, colon);
	const auto port = (colon == std::string_view::npos) ? std::nullopt : parseUnsigned(endpoint.substr(colon + 1));

	if (!isValidHost(host) || !port || *port == 0 || *port > 65535) {
		std::cerr << "FLAPPY_SCOREBOARD should look like host:port, using the default server\n";
		return;
	}
	mHost = host;
	mPort = static_cast<std::uint16_t>(*port);
}

ScoreboardClient::ScoreboardClient(std::string host, std::uint16_t port)
	: mHost(std::move(host)), mPort(port) {}

std::expected<void, std::string> ScoreboardClient::submitScore(std::string_view name, unsigned int score) const {
	if (!isValidName(name))
		return std::unexpected("invalid name");

	const auto response = makeClient(mHost, mPort).Post("/scores", httplib::Params{{"name", std::string(name)}, {"score", std::to_string(score)}});
	if (!response)
		return std::unexpected(httplib::to_string(response.error()));
	if (response->status != 204 && response->status != 200)
		return std::unexpected("server refused score (" + std::to_string(response->status) + ")");
	return {};
}

std::expected<std::vector<ScoreEntry>, std::string> ScoreboardClient::topScores() const {
	const auto response = makeClient(mHost, mPort).Get("/scores");
	if (!response)
		return std::unexpected(httplib::to_string(response.error()));
	if (response->status != 200)
		return std::unexpected("server error (" + std::to_string(response->status) + ")");

	// One "name score" per line, best first.
	std::vector<ScoreEntry> entries;
	std::string_view rest = response->body;
	while (!rest.empty() && entries.size() < SCOREBOARD_TOP_COUNT) {
		const auto newline = rest.find('\n');
		const std::string_view line = rest.substr(0, newline);
		rest = (newline == std::string_view::npos) ? std::string_view{} : rest.substr(newline + 1);

		const auto space = line.find(' ');
		const std::string_view name = line.substr(0, space);
		const auto score = (space == std::string_view::npos) ? std::nullopt : parseUnsigned(line.substr(space + 1));
		if (!isValidName(name) || !score)
			return std::unexpected("bad response from server");
		entries.push_back({std::string(name), *score});
	}
	return entries;
}

bool ScoreboardClient::isValidName(std::string_view name) noexcept {
	const auto isAlnum = [](char c) {
		return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
	};
	return !name.empty() && name.size() <= SCOREBOARD_NAME_MAX && std::ranges::all_of(name, isAlnum);
}
