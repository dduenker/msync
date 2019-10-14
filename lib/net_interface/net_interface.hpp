#ifndef _MSYNC_NET_INTERFACE_HPP_
#define _MSYNC_NET_INTERFACE_HPP_

#include <string_view>

struct net_response
{
	int status_code = 200;
	bool retryable_error = false;
	bool okay = true;
};

using post_request = net_response (const std::string_view url, const std::string_view access_token);

inline constexpr const char* get_error_message(int status_code, bool verbose)
{
	switch (status_code)
	{
	case 200:
	case 201:
	case 204:
		return verbose ? "OK\n" : "";

	case 400:
		return "Bad request. This is probably a programming error. Sorry about that.\n";
	case 403:
		return "Forbidden. Is your access token correct?\n";
	case 404:
		return "Not found, is your instance URL and post ID correct?\n";
	case 408:
		return "Request timed out, but may have still gone through.\n";
	case 429:
		return "Rate limited. Will try again later.\n";

	case 500:
	case 501:
	case 502:
	case 503:
	case 504:
		return "Server error. Try again later.\n";

	default:
		return "[unknown response]\n";
	}

}

#endif