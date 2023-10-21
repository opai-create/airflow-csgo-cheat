#pragma once

namespace netvars
{
	inline std::vector<c_recv_table*> tables;
	extern int get_prop(c_recv_table* table, std::uint32_t prop_name, c_recv_prop** prop = 0);
	extern c_recv_table* get_table(std::uint32_t table_name);
	extern bool hook_prop(std::uint32_t table_name, std::uint32_t prop_name, recv_var_proxy_fn fn, recv_var_proxy_fn* original = nullptr);

	extern std::uint32_t get_offset(std::uint32_t table_hash, std::uint32_t prop_hash);
	void init();
}