#include "global_options.hpp"
#include "user_options.hpp"
#include <filesystem.hpp>
#include <msync_exception.hpp>
#include <print_logger.hpp>
#include <whereami.h>
#include <algorithm>
#include <memory>

#include <cctype>

global_options& options()
{
	static global_options options;
	return options;
}


std::pair<const std::string, user_options>& global_options::add_new_account(std::string name)
{
    fs::path user_path = account_directory_location / name;

    fs::create_directories(user_path); //can throw if something goes wrong

    user_path /= User_Options_Filename;

	const auto [it, inserted] = accounts.emplace(std::move(name), user_options{ std::move(user_path) });

    if (!inserted)
        plverb() << "Account already exists. Nothing changed.\n";

    return *it;
}

fs::path global_options::get_exe_location()
{
    // see https://github.com/gpakosz/whereami
    const int length = wai_getModulePath(nullptr, 0, nullptr);

    auto path = std::make_unique<char[]>(static_cast<size_t>(length) + 1);

    int dirname_length;
    wai_getExecutablePath(path.get(), length, &dirname_length);
    return fs::path(path.get(), path.get() + dirname_length);
}

std::unordered_map<std::string, user_options> global_options::read_accounts()
{
    plverb() << "Reading accounts from " << account_directory_location << "\n";

    std::unordered_map<std::string, user_options> toreturn;

    if (!fs::exists(account_directory_location))
        return toreturn;

    for (const auto& userfolder : fs::directory_iterator(account_directory_location))
    {
        if (!fs::is_directory(userfolder))
        {
            plverb() << userfolder << " is not a directory. Skipping.\n";
            continue;
        }

        fs::path configfile = userfolder / User_Options_Filename;

        if (!fs::exists(configfile))
        {
			using namespace std::string_literals;
            throw msync_exception("Expected to find a config file and didn't find it. Try deleting the folder and running new again: "s + userfolder.path().string());
        }

		toreturn.emplace(userfolder.path().filename().string(), user_options{ std::move(configfile) });
    }

    return toreturn;
}

std::pair<const std::string, user_options>* global_options::select_account(const std::string_view name)
{
	std::pair<const std::string, user_options>* candidate = nullptr;
    for (auto& entry : accounts)
    {
        // if name is longer than the entry, we'll step off the end of entry and segfault
        // since name can't possibly match something it's longer than, just skip this
        if (name.size() > entry.first.size())
            continue;

        // won't have string.starts_with until c++20, so
        // if the name given is a prefix of (or equal to) this entry, it's a candidate
        if (std::equal(name.begin(), name.end(), entry.first.begin(), [](auto a, auto b) {
                return std::tolower(a) == std::tolower(b); //case insensitive
            }))
        {
            plverb() << "Matched account " << entry.first << '\n';

			// if this is the second candidate we've found, it's ambiguous and return nothing
			if (candidate != nullptr) { return nullptr; }

            candidate = &entry;
        }
    }
	
	// nullptr if we found nothing, points to the account entry if we found exactly one candidate
	return candidate;
}

std::vector<std::string_view> global_options::all_accounts() const
{
	std::vector<std::string_view> toreturn(accounts.size());
	std::transform(accounts.begin(), accounts.end(), toreturn.begin(), [](const auto& pair)
		{
			return std::string_view{ pair.first };
		});
	return toreturn;
}
