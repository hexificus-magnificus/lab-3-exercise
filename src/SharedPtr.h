#ifndef SHARED_PTR_HEADER
#define SHARED_PTR_HEADER
#include <utility>

class ControlBlockBase {
public:
    ControlBlockBase() : refcount(0) {} // TODO: implement the default constructor.

    // dtor is virtual, so that we can call derived class's dtor from a ptr to this base class.
    virtual ~ControlBlockBase() {} // TODO: implement the destructor.

    // pure virtual function; must be overriden by derived classes
    virtual void* managedAddress() = 0;

    // Delete copies, which also implicitly deletes moves.
    ControlBlockBase(const ControlBlockBase&) = delete;
    ControlBlockBase& operator=(const ControlBlockBase&) = delete;

    long increment()
    {
        // TODO: increment refcount by 1 and return result.
	return ++refcount;
    }

    long decrement()
    {
        // TODO: decrement refcount by 1 and return result.
	return --refcount;
    }

    long refCount() const
    {
        // TODO: just return the refcount.
	return refcount;
    }

private:
    // TODO: add field(s) which both control block types need to have
    long refcount;
};

template <class T>
class ControlBlock : public ControlBlockBase {
public:
	ControlBlock() {
		mPtr = nullptr;
	}
	ControlBlock(T* p) {
		mPtr = p;
	}
	~ControlBlock() {
		if (mPtr)
			delete mPtr;	
	}

	void* managedAddress() {
		return mPtr;
	}

private:
	T* mPtr;
};

template <class T>
class SharedPtr {
public:
	SharedPtr() : mPtr(nullptr), cBlock(nullptr) {}
	SharedPtr(T* p) : mPtr(p), cBlock(new ControlBlock<T>(p)) {
		if (cBlock)
			cBlock->increment();
	}
	SharedPtr(const SharedPtr<T>& p) : mPtr(p.mPtr), cBlock(p.cBlock) {
		if (cBlock)
			cBlock->increment();
	}
	SharedPtr(SharedPtr<T>&& p) : mPtr(p.mPtr), cBlock(p.cBlock) {
		p.mPtr = nullptr;
		p.cBlock = nullptr;
	}

	template <typename U>
	SharedPtr(const SharedPtr<U>& other, T* storedPtr) : cBlock(other.cBlock), mPtr(storedPtr) {
		if (cBlock)
			cBlock->increment();
	}

	SharedPtr& operator=(const SharedPtr<T>& p) {
		mPtr = p.mPtr;
		cBlock = p.cBlock;
		if (cBlock)
			cBlock->increment();
		return *this;
	}
	SharedPtr& operator=(SharedPtr<T>&& p) {
		if (cBlock && !cBlock->decrement()) {
			delete cBlock;
		}
		mPtr = std::exchange(p.mPtr, nullptr);
		cBlock = std::exchange(p.cBlock, nullptr);
		return *this;
	}

	T& operator*() const {
		return *mPtr;
	}
	T* operator->() const {
		return mPtr;
	}

	T* get() const {
		return mPtr;
	}
	long useCount() const {
		return cBlock->refCount();
	}

	bool operator==(const SharedPtr<T>& other) const {
		return (mPtr == other.mPtr);
	}
	operator bool() const {
		return (mPtr != nullptr);
	}

	void swap(SharedPtr<T>& other) {
		std::swap(mPtr, other.mPtr);
		std::swap(cBlock, other.cBlock);
	}

	void reset() {
		SharedPtr<T> tmp{};
		this->swap(tmp);
	}
	void reset(T* other) {
		SharedPtr<T> tmp{other};
		this->swap(tmp);
	}

	~SharedPtr() {
		if (cBlock && !cBlock->decrement()) {
			delete cBlock;
		}
	}

private:
	T* mPtr;
	ControlBlockBase* cBlock;
};

template <typename T, typename... Args>
SharedPtr<T> makeSharedBasic(Args&&... args) {
	T* tmp = new T(std::forward<Args>(args)...);
	return SharedPtr<T>(tmp);
}

#endif
