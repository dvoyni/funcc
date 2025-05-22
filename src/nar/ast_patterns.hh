#pragma once

#include "../_external.hh"
#include "ast_common.hh"

namespace funcc::nar {
	class PatternAlias final : public IPattern {
		Identifier m_name;
		std::shared_ptr<IPattern> m_nested;

	public:
		PatternAlias(Range range, std::shared_ptr<IType> type, Identifier name, std::shared_ptr<IPattern> nested) :
			IPattern(range, std::move(type)),
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
		PatternAny(Range range, std::shared_ptr<IType> type) :
			IPattern(range, std::move(type)) {}

		~PatternAny() override = default;

		[[nodiscard]] bool IsAny() const {
			return true;
		}
	};

	class PatternCons final : public IPattern {
		std::shared_ptr<IPattern> m_head;
		std::shared_ptr<IPattern> m_tail;

	public:
		PatternCons(
			Range range,
			std::shared_ptr<IType> type,
			std::shared_ptr<IPattern> head,
			std::shared_ptr<IPattern> tail
		) :
			IPattern(range, std::move(type)),
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
		PatternConst(Range range, std::shared_ptr<IType> type, std::shared_ptr<IConst> value) :
			IPattern(range, std::move(type)),
			m_value(std::move(value)) {}

		~PatternConst() override = default;

		[[nodiscard]] IConst const& GetValue() const {
			return *m_value;
		}
	};

	class PatternNamed final : public IPattern {
		Identifier m_name;

	public:
		PatternNamed(Range range, std::shared_ptr<IType> type, Identifier name) :
			IPattern(range, std::move(type)),
			m_name(std::move(name)) {}

		~PatternNamed() override = default;

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}
	};

	class PatternDataConstructor final : public IPattern {
		Identifier m_name;
		Range m_nameRange;
		std::vector<std::shared_ptr<IPattern>> m_values;

	public:
		PatternDataConstructor(
			Range range,
			std::shared_ptr<IType> type,
			Identifier name,
			Range nameRange,
			std::vector<std::shared_ptr<IPattern>>&& values
		) :
			IPattern(range, std::move(type)),
			m_name(std::move(name)),
			m_nameRange(nameRange),
			m_values(std::move(values)) {}

		~PatternDataConstructor() override = default;

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Range const& GetNameRange() const {
			return m_nameRange;
		}

		[[nodiscard]] std::vector<std::shared_ptr<IPattern>> const& GetValues() const {
			return m_values;
		}
	};

	class PatternList final : public IPattern {
		std::vector<std::shared_ptr<IPattern>> m_patterns;

	public:
		PatternList(Range range, std::shared_ptr<IType> type, std::vector<std::shared_ptr<IPattern>>&& patterns) :
			IPattern(range, std::move(type)),
			m_patterns(std::move(patterns)) {}

		~PatternList() override = default;

		[[nodiscard]] std::vector<std::shared_ptr<IPattern>> const& GetPatterns() const {
			return m_patterns;
		}
	};

	class PatternRecord final : public IPattern {
		std::vector<std::pair<Range, Identifier>> m_fields;

	public:
		PatternRecord(Range range, std::shared_ptr<IType> type, std::vector<std::pair<Range, Identifier>>&& fields) :
			IPattern(range, std::move(type)),
			m_fields(std::move(fields)) {}

		~PatternRecord() override = default;

		[[nodiscard]] std::vector<std::pair<Range, Identifier>> const& GetFields() const {
			return m_fields;
		}
	};

	class PatternTuple final : public IPattern {
		std::vector<std::shared_ptr<IPattern>> m_items;

	public:
		PatternTuple(Range range, std::shared_ptr<IType> type, std::vector<std::shared_ptr<IPattern>>&& items) :
			IPattern(range, std::move(type)),
			m_items(std::move(items)) {}

		~PatternTuple() override = default;

		[[nodiscard]] std::vector<std::shared_ptr<IPattern>> const& GetItems() const {
			return m_items;
		}
	};
}
