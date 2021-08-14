#include "BasicTagTypes.h"
#include "TagInterface.h"

int32_t tag_block_add_impl(tag_block_ref* block) {
	return tags::add_block_element(block);
}
