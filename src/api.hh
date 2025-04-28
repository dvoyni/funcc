#pragma once

#include <string>

#include "ast_normalized.hh"
#include "helpers.hh"

namespace funcc {

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
