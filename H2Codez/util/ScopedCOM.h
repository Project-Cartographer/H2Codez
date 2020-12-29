#pragma once

template<typename t>
class ScopedCOM {
	t* data = nullptr;

public:
	~ScopedCOM() {
		if (data)
			data->Release();
	}

	ScopedCOM() = default;

	ScopedCOM(const ScopedCOM& other) {
		data = other.data;
		if (data)
			data->AddRef();
	}

	bool is_valid() const {
		return data != nullptr;
	}

	bool operator !() const {
		return !is_valid();
	}

	t** operator &() {
		return &data;
	}

	t* operator->() {
		return get();
	}

	t* get() {
		return data;
	}
};