#ifndef EMP_BIT_H__
#define EMP_BIT_H__
#include "emp/gc/backend.h"
#include "emp/utils/utils.h"
#include "emp/garble/block.h"
#include "swappable.h"

class Bit : public Swappable<Bit>{ public:
	block bit;

	Bit(bool _b = false, int party = PUBLIC);
	Bit(const block& a) {
		memcpy(&bit, &a, sizeof(block));
	}

	template<typename O = bool> 
	O reveal(int party = PUBLIC) const;

	Bit operator!=(const Bit& rhs) const; 
	Bit operator==(const Bit& rhs) const;
	Bit operator &(const Bit& rhs) const;  
	Bit operator |(const Bit& rhs) const;
	Bit operator !() const; 

	//swappable
	Bit select(const Bit & select, const Bit & new_v)const ;
	Bit operator ^(const Bit& rhs) const;

	//batcher
	template<typename... Args>
	static size_t bool_size(Args&&... args) {
		return 1;
	}

	static void bool_data(bool *b, bool data) {
		b[0] = data;
	}

	Bit(size_t size, const block* a) {
		memcpy(&bit, a, sizeof(block));
	}
};
#include "bit.hpp"
#endif
