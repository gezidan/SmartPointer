/*
* strong_ptr - simple reference counted pointer.
*
* The is a non-intrusive implementation that allocates an additional
* int and pointer for every counted object.
*/

#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

namespace smart_ptr {

}; // namespace smart_ptr

class ref_count
{
public:
    ref_count() : m_strong_ref_count(1), m_weak_ref_count(0)
    {
    }

    ~ref_count()
    {
    }

    // increment use count
    int inc_ref()
    {
        return ++m_strong_ref_count;
    }

    // increment weak reference count
    int inc_weak_ref()
    {
        return ++m_weak_ref_count;
    }

    // decrement use count
    int dec_ref()
    {
        int nRs = 0;
        if (m_strong_ref_count > 0) {
            nRs = --m_strong_ref_count;
        }
        return nRs;
    }

    // decrement weak reference count
    int dec_weak_ref()
    {
        int nRs = 0;
        if (m_weak_ref_count > 0) {
            nRs = --m_weak_ref_count;
        }
        return nRs;
    }

    // return use count
    int get_ref_count() const
    {
        return m_strong_ref_count;
    }

    // return true if _Uses == 0
    bool expired() const
    {
        return (get_ref_count() == 0);
    }

    int get_weak_ref_count() const
    {
        return m_weak_ref_count;
    }

private:
    int m_strong_ref_count;
    int m_weak_ref_count;
}; 


// base class for strong_ptr and weak_ptr
template<class X, bool isStrong>
class base_ptr
{
public:
    base_ptr(X *p=0) : m_counter(0), m_ptr(p)
    {
        if (m_ptr) {
            if (isStrong) {
                // allocate a new ref_count
                m_counter = new ref_count;
            }
        }
    }

    virtual ~base_ptr()
    {
        release();
    }

    X& operator*()  const throw()   { return *m_ptr; }
    X* operator->() const throw()   { return m_ptr; }
    X* get()        const throw()   { return m_ptr; }

    bool unique() const throw()
    { return (m_counter ? (1 == m_counter->get_ref_count()) : true); }

    void reset(X *p=0)
    {
        base_ptr<X, isStrong> ptr(p);
        reset(ptr);
    }

    template <class Y, bool b> void reset(const base_ptr<Y, b> &rhs)
    {
        if ((void *)this != (void *)&rhs) {
            release();
            acquire(rhs);
        }
    }

    int use_count(void) const
    {
        int nRs = 0;
        if (m_counter) {
            nRs = m_counter->get_ref_count();
        }
        return nRs;
    }

    // swap pointers
    template <class Y, bool b> void swap(base_ptr<Y, b> & rhs)
    {
        private_swap(m_counter, rhs.m_counter);
        private_swap(m_ptr, rhs.m_ptr);
    }

protected:
    ref_count *m_counter;
    X * m_ptr;

    template <typename TP1, typename TP2>
    static void private_swap(TP1 &obj1, TP2 &obj2)
    {
        TP1 tmp = obj1;
        obj1 = static_cast<TP1>(obj2);
        obj2 = static_cast<TP2>(tmp);
    }

    template <class Y, bool b> void acquire(const base_ptr<Y, b> & c) throw()
    {
        m_ptr = static_cast<X*>(c.m_ptr);
        m_counter = c.m_counter;
        if (m_counter) {
            if (isStrong) {
                m_counter->inc_ref();
            } else {
                m_counter->inc_weak_ref();
            }
        }
    }

    // decrement the count, delete if it is 0
    void release(void)
    {
        if (m_counter) {
            if (isStrong) {
                if (0 == m_counter->dec_ref()) {
                    delete m_ptr;
                    m_ptr = 0;
                }
            } else {
                m_counter->dec_weak_ref();
            }
            if (0 == m_counter->get_ref_count() && 0==m_counter->get_weak_ref_count()) {
                delete m_counter;
                m_counter = 0;
            }
        }
    }

    template<class Y, bool b> friend class base_ptr;
    template<class Y, bool b> friend class base_ptr;
};

template <class X> class weak_ptr;

template <class X> class strong_ptr : public base_ptr<X, true>
{
public:
    explicit strong_ptr(X* p = 0) : base_ptr(p)
    {
    }

    strong_ptr(const strong_ptr& r)
    {
        acquire(r);
    }

    template<class Y> strong_ptr(const strong_ptr<Y> &rhs)
    {
        acquire(rhs);
    }

     // construct strong_ptr object that owns resource *rhs
     template<class Y> explicit strong_ptr(const weak_ptr<Y> &rhs)
     {
         acquire(rhs);
     }

    ~strong_ptr()
    {
    }

    strong_ptr& operator=(const strong_ptr& rhs)
    {
        reset(rhs);
        return *this;
    }

    template <class Y> strong_ptr& operator=(const strong_ptr<Y>& rhs)
    {
        reset(rhs);
        return *this;
    }
};


template<class _Ty1, class _Ty2>
bool operator<(const strong_ptr<_Ty1> &lhs, const strong_ptr<_Ty2> &rhs)
{
    // test if base_ptr < base_ptr
    return lhs.get() < rhs.get();
}


template <class X> class weak_ptr : public base_ptr<X, false>
{
public:
    // construct empty weak_ptr object
    weak_ptr()
    {
    }

    // construct weak_ptr object for resource owned by rhs
    template<class Y> weak_ptr(const strong_ptr<Y> &rhs)
    {
        acquire(rhs);
    }

    // construct weak_ptr object for resource pointed to by rhs
    weak_ptr(const weak_ptr &rhs)
    {
        acquire(rhs);
    }

    // construct weak_ptr object for resource pointed to by rhs
    template<class Y> weak_ptr(const weak_ptr<Y> &rhs)
    {
        acquire(rhs);
    }

    ~weak_ptr()
    {
    }

    weak_ptr& operator=(const weak_ptr &rhs)
    {
        reset(rhs)
        return *this;
    }

    template <class Y> weak_ptr& operator=(const weak_ptr<Y>& rhs)
    {
        reset(rhs);
        return *this;
    }

    template<class Y> weak_ptr& operator=(const strong_ptr<Y> &rhs)
    {
        reset(rhs);
        return *this;
    }

    // return true if resource no longer exists
    bool expired() const
    {
        return m_counter ? m_counter->expired() : true;
    }

    // convert to strong_ptr
    strong_ptr<X> lock() const
    {
        return strong_ptr<X>(*this);
    }
};

template<class _Ty1, class _Ty2>
bool operator<(const weak_ptr<_Ty1> &lhs, const weak_ptr<_Ty2> &rhs)
{
    // test if base_ptr < base_ptr
    return lhs.get() < rhs.get();
}



#endif // __SMART_PTR_H__
