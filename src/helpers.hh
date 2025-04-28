#pragma once

#include <any>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "ast_common.hh"

namespace funcc {
	class Context {
		std::unordered_map<std::type_index, std::any> m_context;

	public:
		Context() = default;
		virtual ~Context() = default;

		template<typename T>
		void set(T&& value) {
			m_context[typeid(T)] = std::forward<T>(value);
		};

		template<typename T>
		T& get() const {
			auto it = m_context.find(typeid(T));
			if (it == m_context.end()) {
				throw std::runtime_error("Context does not contain value of type " + std::string(typeid(T).name()));
			}
			return std::any_cast<T&>(it->second);
		};
	};

	template<typename T>
	class Result final {
		Location m_location;
		std::string m_error;
		std::shared_ptr<T> m_value;

	public:
		constexpr Result() :
			m_location{0, 0},
			m_error{""},
			m_value{} {};

		template<typename... Args>
		explicit Result(Args... args) :
			m_location{0, 0},
			m_error{""},
			m_value{std::make_shared<T>(args...)} {}

		Result(std::string const& error, size_t position, Location const& location) :
			m_location{std::move(location)},
			m_error{std::move(error)},
			m_value{} {}

		[[nodiscard]] bool IsError() const {
			return m_location.line != 0;
		}

		[[nodiscard]] std::shared_ptr<T> const& GetValue() const {
			assert(!IsError());
			return m_value;
		}

		[[nodiscard]] std::string const& GetError() const {
			assert(IsError());
			return m_error;
		}

		[[nodiscard]] Location const& GetLocation() const {
			assert(IsError());
			return m_location;
		}

		[[nodiscard]] operator bool() const {
			return !IsError();
		}

		[[nodiscard]] T const* operator->() const {
			assert(!IsError());
			return *m_value;
		}

		[[nodiscard]] operator std::shared_ptr<T>() {
			assert(!IsError());
			return m_value;
		}

		bool operator<(Result<T> const& other) {
			return m_location.line == other.m_location.line
				? m_location.column < other.m_location.column
				: m_location.line < other.m_location.line;
		}
	};
}
