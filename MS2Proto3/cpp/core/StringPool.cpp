// StringPool.cpp
#include "StringPool.h"
#include "CS_String.h"
#include "hashing.h"
#include <cstring>  // strlen, memcpy
#include <cstdio>

uint8_t String::defaultPool = 0;

namespace StringPool {

static Pool pools[256];  // 256 pools because poolNum is a utf8_t

// Small helpers to keep code tidy
static inline HashEntry* derefHE(MemRef r) {
    return (HashEntry*)MemPoolManager::getPtr(r);
}

static int utf8_char_count(const char* s, int n) {
    int c = 0; for (int i = 0; i < n; ++i) if ((s[i] & 0xC0) != 0x80) ++c; return c;
}

// StringStorage allocated inside the MemPool with the same number as our StringPool
static MemRef allocStringStorage(uint8_t poolNum, const char* src, int lenB, uint32_t hash) {
    MemRef r = MemPoolManager::alloc(sizeof(StringStorage) + lenB + 1, poolNum);
    if (r.isNull()) return r;
    StringStorage* ss = (StringStorage*)MemPoolManager::getPtr(r);
    ss->lenB = lenB;
    ss->lenC = utf8_char_count(src, lenB);
    ss->hash = hash;
    if (lenB) memcpy(ss->data, src, lenB);
    ss->data[lenB] = '\0';
    return r;
}

static void initPool(uint8_t poolNum) {
    Pool& p = pools[poolNum];
    if (p.initialized) return;
    p.initialized = true;
    for (int i = 0; i < 256; ++i) p.hashTable[i] = MemRef{}; // null

	// add empty string to hash table
    uint32_t h = string_hash("", 0);
    MemRef emptyRef = allocStringStorage(poolNum, "", 0, h);
    
	uint8_t b = (uint8_t)(h & 0xFF);
    MemRef heRef = MemPoolManager::alloc(sizeof(HashEntry), poolNum);
    HashEntry* he = derefHE(heRef);
    he->hash = h; he->ssRef = emptyRef; he->next = p.hashTable[b];
    p.hashTable[b] = heRef;
}

// Look for an existing interned string that exactly matches ss.
// If found, free ss, and return the found index.
// Otherwise, adopt ss into our mem pool.
// The given StringStorage must have been allocated with malloc,
// and the caller must not free it.
MemRef internOrAdoptString(uint8_t poolNum, StringStorage *ss) {
	if (!ss) return MemRef::Null;
	initPool(poolNum);
	Pool& p = pools[poolNum];
	
	int lenB = ss_lengthB(ss);
	if (lenB == 0) return MemRef::Null;
	
	const char *cstr = ss_getCString(ss);
	if (ss->hash == 0) ss->hash = string_hash(cstr, lenB);
	uint32_t h = ss->hash;
	uint8_t b = (uint8_t)(h & 0xFF);
	
	// lookup via MemRef chain
	for (MemRef eRef = p.hashTable[b]; !eRef.isNull(); eRef = derefHE(eRef)->next) {
		HashEntry* e = derefHE(eRef);
		if (e->hash != h) continue;
		const StringStorage* s = e->stringStorage();
		if (s && s->lenB == lenB && memcmp(s->data, cstr, lenB) == 0) {
			// Found a match!  Free the given ss, and return the index.
			free(ss);
			return e->ssRef;
		}
	}
	
	// No match found -- add ss to our hash table, adopting it.
	MemRef sRef(poolNum, MemPoolManager::getPool(poolNum)->adopt(ss, ss_totalSize(ss)));
	return storeInPool(sRef, poolNum, h);
}

// Copy and intern a C string into our string pool.
MemRef internString(uint8_t poolNum, const char* cstr) {
    if (!cstr) return MemRef::Null;
    initPool(poolNum);
    Pool& p = pools[poolNum];
	
    int lenB = (int)strlen(cstr);
    if (lenB == 0) return MemRef::Null;

    uint32_t h = string_hash(cstr, lenB);
    uint8_t b = (uint8_t)(h & 0xFF);

    // lookup via MemRef chain
    for (MemRef eRef = p.hashTable[b]; !eRef.isNull(); eRef = derefHE(eRef)->next) {
        HashEntry* e = derefHE(eRef);
        if (e->hash != h) continue;
		const StringStorage* s = e->stringStorage();
        if (s && s->lenB == lenB && memcmp(s->data, cstr, lenB) == 0) {
            return e->ssRef;
        }
    }

    // create new StringStorage in this MemPool
    MemRef sRef = allocStringStorage(poolNum, cstr, lenB, h);
	return storeInPool(sRef, poolNum, h);
}

// Helper function to store a StringStorage (already wrapped in a MemRef) in
// our hash map and mem pool, returning its index (in the StringPool).
MemRef storeInPool(MemRef sRef, uint8_t poolNum, uint32_t hash) {
	if (sRef.isNull()) return MemRef::Null;

	// add hash entry
	Pool& p = pools[poolNum];
	MemRef heRef = MemPoolManager::alloc(sizeof(HashEntry), poolNum);
	HashEntry* ne = derefHE(heRef);
	uint8_t b = (uint8_t)(hash & 0xFF);
	ne->hash = hash; ne->ssRef = sRef; ne->next = p.hashTable[b];
	p.hashTable[b] = heRef;

	return sRef;
}

void clearPool(uint8_t poolNum) {
    // Reset StringPool state first
    Pool& p = pools[poolNum];
    if (p.initialized) {
        
        // Clear the Pool struct
        p.initialized = false;
        
        // Clear hash table
        for (int i = 0; i < 256; ++i) {
            p.hashTable[i] = MemRef();  // null
        }
    }
    
    // Now clear the underlying MemPool
    MemPoolManager::clearPool(poolNum);
}

StringStorage* defaultStringAllocator(const char* src, int lenB, uint32_t hash) {
    // Choose a default pool (0) or make this configurable if you like.
    MemRef r = MemPoolManager::alloc(sizeof(StringStorage) + lenB + 1, /*pool*/0);
    if (!r.isNull()) return nullptr;
    StringStorage* ss = (StringStorage*)MemPoolManager::getPtr(r);
    ss->lenB = lenB;
    ss->lenC = utf8_char_count(src, lenB);
    ss->hash = hash;
    if (lenB) memcpy(ss->data, src, lenB);
    ss->data[lenB] = '\0';
    return ss;
}

void* poolAllocator(size_t size) {
    // Use pool 0 by default
    MemRef r = MemPoolManager::alloc(size, 0);
    return r.isNull() ? nullptr : MemPoolManager::getPtr(r);
}

void* poolAllocatorForPool(size_t size, uint8_t poolNum) {
    MemRef r = MemPoolManager::alloc(size, poolNum);
    return r.isNull() ? nullptr : MemPoolManager::getPtr(r);
}

} // namespace StringPool

// C-compatible wrapper for the pool allocator
extern "C" void* stringpool_allocator(size_t size) {
    return StringPool::poolAllocator(size);
}
