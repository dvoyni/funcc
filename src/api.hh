#pragma once

#include <string>

#include "ast_normalized.hh"
#include "helpers.hh"

namespace funcc {
	template<typename T>
	struct Result {
		bool ok;
		Range range;
		T value;
		std::string message;

	private:
		Result(bool ok, Range const& range, T const& value, std::string const& message) :
			ok(ok),
			range(range),
			value(value),
			message(message) {}

	public:
		Result() = default;

		inline static Result Ok(Range const& range, T const& value) {
			return {true, range, value, ""};
		}

		inline static Result Err(Range const& range, std::string const& message) {
			return {false, range, T{}, message};
		}
	};

	class IParsedFile {
	public:
		virtual ~IParsedFile() = default;

		virtual Result<normalized::File> Normalize(Context const& ctx) const = 0;
	};

	class IParser {
	public:
		virtual ~IParser() = default;

		/**
		 * @brief
		 *
		 * @param ctx
		 * @param text
		 * @return
		 *
		 * @throws ParseError
		 */
		virtual IParsedFile Parse(Context const& ctx, std::string const& text) = 0;
	};
}
