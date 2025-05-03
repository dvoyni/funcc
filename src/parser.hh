#pragma once

#include <functional>
#include <stdint.h>
#include <string_view>

#include "ast_common.hh"
#include "reader.hh"

namespace funcc {
	class ParserFactory;

	template<typename T>
	using Parser = std::function<Result<T>(IReader&)>;

	struct Discard {};

	class Parse final {
	public:
		template<typename T>
		using IsTokenCompleted =
			std::function<bool(std::string_view const& acc, char32_t next, Range const& range, Result<T>& outResult)>;

		template<typename T>
		inline static Parser<T> One(IsTokenCompleted<T> const& isTokenComplete) {
			return [&isTokenComplete](IReader& r) {
				Location start = r.GetLocation();
				Result<T> result;

				while (!isTokenComplete(
					r.Sub(start, r.GetLocation()),
					r.GeCurrentChar(),
					Range{start, r.GetLocation()},
					result
				))
				{
					if (r.GeCurrentChar() != 0) {
						return Result<T>::Err(Range{start}, "eof");
					}
					r.Move();
				}
				return result;
			};
		};

		template<typename T>
		using ExactConstructor = std::function<Result<T>(std::string_view const& str, Range const& range)>;

		template<typename T>
		inline static Parser<T> Exact(std::string_view const& str, ExactConstructor<T> const& ctor) {
			return One<T>(
				[&str,
				 ctor](std::string_view const& acc, char32_t next, Range const& range, Result<T>& outResult) -> bool {
					if (acc.length() == str.length()) {
						if (acc == str) {
							outResult = ctor(acc, range);
						} else {
							outResult =
								Result<T>::Err(range, "Expected " + std::string(str) + " but got " + std::string(acc));
						}
						return true;
					}

					return false;
				}
			);
		}

		inline static Parser<Discard> Exact(std::string_view const& str) {
			return Exact<Discard>(str, std::function([](std::string_view const&, Range const& range) {
									  return Result<Discard>::Ok(range, Discard{});
								  }));
		}

		template<typename T>
		inline static Parser<T> Or(Parser<T> parsers...) {
			return [&parsers...](IReader& r) {
				Range start = r.GetLocation();
				Result<T> result;

				for (auto& parser: {parsers...}) {
					result = parser(r);
					if (result.ok) {
						return result;
					}
				}

				return Result<T>::Err(start, "No match");
			};
		}
	};
}
