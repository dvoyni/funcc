#pragma once

#include <any>
#include <cassert>
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
				throw std::exception("Context does not contain value of type " + std::string(typeid(T).name()));
			}
			return std::any_cast<T&>(it->second);
		};
	};
}
