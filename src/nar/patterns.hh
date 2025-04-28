#pragma once

#include "ast.hh"

namespace funcc::nar {
	class PatternAlias final : public IPattern {
		Identifier m_name;
		std::unique_ptr<IPattern> m_nested;

	public:
		PatternAlias(
			Location location,
			std::unique_ptr<IType> type,
			Identifier name,
			std::unique_ptr<IPattern> nested
		) :
			IPattern(location, std::move(type)),
			m_name(std::move(name)),
			m_nested(std::move(nested)) {}

		~PatternAlias() override = default;

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] IPattern const& GetNested() const {
			return *m_nested;
		}
	};

	class PatternAny final : public IPattern {
	public:
		PatternAny(Location location, std::unique_ptr<IType> type) :
			IPattern(location, std::move(type)) {}

		~PatternAny() override = default;

		[[nodiscard]] bool IsAny() const {
			return true;
		}
	};

	class PatternCons final : public IPattern {
		std::unique_ptr<IPattern> m_head;
		std::unique_ptr<IPattern> m_tail;

	public:
		PatternCons(
			Location location,
			std::unique_ptr<IType> type,
			std::unique_ptr<IPattern> head,
			std::unique_ptr<IPattern> tail
		) :
			IPattern(location, std::move(type)),
			m_head(std::move(head)),
			m_tail(std::move(tail)) {}

		~PatternCons() override = default;

		[[nodiscard]] IPattern const& GetHead() const {
			return *m_head;
		}

		[[nodiscard]] IPattern const& GetTail() const {
			return *m_tail;
		}
	};

	class PatternConst final : public IPattern {
		std::shared_ptr<IConst> m_value;

	public:
		PatternConst(Location location, std::unique_ptr<IType> type, std::shared_ptr<IConst> value) :
			IPattern(location, std::move(type)),
			m_value(std::move(value)) {}

		~PatternConst() override = default;

		[[nodiscard]] IConst const& GetValue() const {
			return *m_value;
		}
	};

	class PatternNamed final : public IPattern {
		Identifier m_name;

	public:
		PatternNamed(Location location, std::unique_ptr<IType> type, Identifier name) :
			IPattern(location, std::move(type)),
			m_name(std::move(name)) {}

		~PatternNamed() override = default;

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}
	};

	class PatternDataConstructor final : public IPattern {
		Identifier m_name;
		Location m_nameLocation;
		std::vector<std::unique_ptr<IPattern>> m_values;

	public:
		PatternDataConstructor(
			Location location,
			std::unique_ptr<IType> type,
			Identifier name,
			Location nameLocation,
			std::vector<std::unique_ptr<IPattern>>&& values
		) :
			IPattern(location, std::move(type)),
			m_name(std::move(name)),
			m_nameLocation(nameLocation),
			m_values(std::move(values)) {}

		~PatternDataConstructor() override = default;

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Location const& GetNameLocation() const {
			return m_nameLocation;
		}

		[[nodiscard]] std::vector<std::unique_ptr<IPattern>> const& GetValues() const {
			return m_values;
		}
	};

	class PatternList final : public IPattern {
		std::vector<std::unique_ptr<IPattern>> m_patterns;

	public:
		PatternList(Location location, std::unique_ptr<IType> type, std::vector<std::unique_ptr<IPattern>>&& patterns) :
			IPattern(location, std::move(type)),
			m_patterns(std::move(patterns)) {}

		~PatternList() override = default;

		[[nodiscard]] std::vector<std::unique_ptr<IPattern>> const& GetPatterns() const {
			return m_patterns;
		}
	};

	class PatternRecord final : public IPattern {
		std::vector<std::pair<Location, Identifier>> m_fields;

	public:
		PatternRecord(
			Location location,
			std::unique_ptr<IType> type,
			std::vector<std::pair<Location, Identifier>>&& fields
		) :
			IPattern(location, std::move(type)),
			m_fields(std::move(fields)) {}

		~PatternRecord() override = default;

		[[nodiscard]] std::vector<std::pair<Location, Identifier>> const& GetFields() const {
			return m_fields;
		}
	};

	class PatternTuple final : public IPattern {
		std::vector<std::unique_ptr<IPattern>> m_items;

	public:
		PatternTuple(Location location, std::unique_ptr<IType> type, std::vector<std::unique_ptr<IPattern>>&& items) :
			IPattern(location, std::move(type)),
			m_items(std::move(items)) {}

		~PatternTuple() override = default;

		[[nodiscard]] std::vector<std::unique_ptr<IPattern>> const& GetItems() const {
			return m_items;
		}
	};
}
