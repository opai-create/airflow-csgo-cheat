#include "globals.hpp"

namespace netvars
{
	INLINE int get_prop(c_recv_table* table, std::uint32_t prop_hash, c_recv_prop** prop)
	{
		if (table == nullptr)
			return 0;

		int offset = 0;
		for (int i = 0; i < table->props_count; ++i)
		{
			auto recv_prop = &table->props[i];
			auto child = recv_prop->data_table;

			if (child != nullptr && child->props_count > 0)
			{
				int diff = get_prop(child, prop_hash, prop);
				if (diff)
					offset += recv_prop->offset + diff;
			}

			if (CONST_HASH(recv_prop->prop_name) != prop_hash)
				continue;

			if (prop != nullptr)
				*prop = recv_prop;

			return recv_prop->offset + offset;
		}

		return offset;
	}

	INLINE c_recv_table* get_table(std::uint32_t table_hash)
	{
		if (tables.empty())
			return nullptr;

		for (auto table : tables)
		{
			if (table == nullptr)
				continue;

			if (CONST_HASH(table->table_name) == table_hash)
				return table;
		}

		return nullptr;
	}

	INLINE bool hook_prop(std::uint32_t table_hash, std::uint32_t prop_hash, recv_var_proxy_fn fn, recv_var_proxy_fn* original)
	{
		auto table = get_table(table_hash);
		if (!table)
			return false;

		c_recv_prop* prop = nullptr;
		get_prop(table, prop_hash, &prop);

		if (prop == nullptr)
			return false;

		if (original)
			*original = prop->proxy_fn;

		prop->proxy_fn = fn;
		return true;
	}

	INLINE std::uint32_t get_offset(std::uint32_t table_hash, std::uint32_t prop_hash)
	{
		return get_prop(get_table(table_hash), prop_hash);
	}

	void init()
	{
		tables.clear();

		auto classes = HACKS->client->get_client_classes();
		if (classes == nullptr)
			return;

		while (classes != nullptr)
		{
			tables.push_back(classes->recvtable_ptr);
			classes = classes->next_ptr;
		}
	}
}