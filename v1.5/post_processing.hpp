#pragma once

class c_post_processing
{
private:
	bool override_processing{};
	bool override_shadow{};
	bool override_viewmodel_move{};

	std::uint32_t m_bUseCustomAutoExposureMin{};
	std::uint32_t m_bUseCustomAutoExposureMax{};
	std::uint32_t m_flCustomAutoExposureMin{};
	std::uint32_t m_flCustomAutoExposureMax{};
	std::uint32_t m_bUseCustomBloomScale{};
	std::uint32_t m_flCustomBloomScale{};

	INLINE bool should_override_processing()
	{
		if (g_cfg.misc.custom_bloom)
			return false;

		if (g_cfg.misc.removals & post_process)
			return true;

		return false;
	}

public:
	INLINE void reset()
	{
		override_processing = false;
		override_shadow = false;
		override_viewmodel_move = false;
	}

	void init();
	void override_fog();
	void remove_bloom();
	void remove();
};

#ifdef _DEBUG
inline auto POST_PROCESSING = std::make_unique<c_post_processing>();
#else
CREATE_DUMMY_PTR(c_post_processing);
DECLARE_XORED_PTR(c_post_processing, GET_XOR_KEYUI32);

#define POST_PROCESSING XORED_PTR(c_post_processing)
#endif