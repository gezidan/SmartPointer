/*
* strong_ptr - simple reference counted pointer.
*
* Copyright (c) 2013, Ralph Shane <free2000fly at gmail dot com>
* 
* The is a non-intrusive implementation that allocates an additional
* int and pointer for every counted object.
*/

#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

namespace smart_ptr {

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
template<class X, bool isStrong, typename mem_mgr>
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

    base_ptr(const base_ptr& rhs) : m_counter(0), m_ptr(0)
    {
        acquire(rhs);
    }

    template<class Y, bool b, typename mem_mgr2> base_ptr(const base_ptr<Y, b, mem_mgr2> &rhs) : m_counter(0), m_ptr(0)
    {
        acquire(rhs);
    }

    virtual ~base_ptr()
    {
        release();
    }

    operator X*()   const throw()   { return m_ptr; }
    X& operator*()  const throw()   { return *m_ptr; }
    X* operator->() const throw()   { return m_ptr; }
    X* get()        const throw()   { return m_ptr; }

    bool unique() const throw()
    { return (m_counter ? (1 == m_counter->get_ref_count()) : true); }

    void reset(X *p=0)
    {
        base_ptr<X, isStrong, mem_mgr> ptr(p);
        reset(ptr);
    }

    template <class Y, bool b, typename mem_mgr2> void reset(const base_ptr<Y, b, mem_mgr2> &rhs)
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
    template <class Y, bool b, typename mem_mgr2> void swap(base_ptr<Y, b, mem_mgr2> & rhs)
    {
        private_swap(m_counter, rhs.m_counter);
        private_swap(m_ptr, rhs.m_ptr);
    }

    base_ptr& operator=(const base_ptr &rhs)
    {
        reset(rhs);
        return *this;
    }

    template <class Y, bool b, typename mem_mgr2> base_ptr& operator=(const base_ptr<Y, b, mem_mgr2> &rhs)
    {
        reset(rhs);
        return *this;
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

    template <class Y, bool b, typename mem_mgr2> void acquire(const base_ptr<Y, b, mem_mgr2> & rhs) throw()
    {
        if (rhs.m_counter && rhs.m_counter->get_ref_count()) {
            m_counter = rhs.m_counter;
            if (isStrong) {
                m_counter->inc_ref();
            } else {
                m_counter->inc_weak_ref();
            }
            m_ptr = static_cast<X*>(rhs.m_ptr);
        }
    }

    // decrement the count, delete if it is 0
    void release(void)
    {
        if (m_counter) {
            if (isStrong) {
                if (0 == m_counter->dec_ref()) {
                    mem_mgr::deallocate(m_ptr);
                    m_ptr = 0;
                }
            } else {
                m_counter->dec_weak_ref();
            }
            if (0 == m_counter->get_ref_count() && 0==m_counter->get_weak_ref_count()) {
                delete m_counter;
            }
            m_counter = 0;
        }
        if (m_ptr) {
            m_ptr = 0;
        }
    }

    template<class Y, bool b, typename mem_mgr2> friend class base_ptr;
};

template<class X, bool bx, class Y, bool by, typename mem_mgr1, typename mem_mgr2>
bool operator<(const base_ptr<X, bx, mem_mgr1> &lhs, const base_ptr<Y, by, mem_mgr2> &rhs)
{
    // test if left pointer < right pointer
    return lhs.get() < rhs.get();
}

template <class X, typename mem_mgr> class weak_ptr;

template<typename X>
class cpp_mem_mgr {
public:
    static void deallocate(X *p) { delete p; }
    static X * allocate(void) { return new X(); }
    template<typename A1> static X * allocate(A1 const &a1) { return new X(a1); }
    template<typename A1, typename A2> static X * allocate(A1 const &a1, A2 const &a2) { return new X(a1, a2); }
    template<typename A1, typename A2, typename A3> static X * allocate(A1 const &a1, A2 const &a2, A3 const &a3) { return new X(a1, a2, a3); }
    template<typename A1, typename A2, typename A3, typename A4> static X * allocate(A1 const &a1, A2 const &a2, A3 const &a3, A4 const &a4) { return new X(a1, a2, a3, a4); }
    template<typename A1, typename A2, typename A3, typename A4, typename A5> static X * allocate(A1 const &a1, A2 const &a2, A3 const &a3, A4 const &a4, A5 const &a5) { return new X(a1, a2, a3, a4, a5); }
    template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6> static X * allocate(A1 const &a1, A2 const &a2, A3 const &a3, A4 const &a4, A5 const &a5, A6 const &a6) { return new X(a1, a2, a3, a4, a5, a6); }
};

template <class X, typename mem_mgr=cpp_mem_mgr<X> > class strong_ptr : public base_ptr<X, true, mem_mgr>
{
    typedef base_ptr<X, true, mem_mgr> baseClass;
public:
    explicit strong_ptr(X* p = 0) : baseClass(p)
    {
    }

    strong_ptr(const strong_ptr& rhs) : baseClass(rhs)
    {
    }

    template<class Y, typename mem_mgr2> strong_ptr(const strong_ptr<Y, mem_mgr2> &rhs) : baseClass(rhs)
    {
    }

    // construct strong_ptr object that owns resource *rhs
    template<class Y, typename mem_mgr2> explicit strong_ptr(const weak_ptr<Y, mem_mgr2> &rhs) : baseClass(rhs)
    {
    }

    ~strong_ptr()
    {
    }

    strong_ptr& operator=(const strong_ptr &rhs)
    {
        baseClass::operator = (rhs);
        return *this;
    }

    template <class Y, typename mem_mgr2> strong_ptr& operator=(const strong_ptr<Y, mem_mgr2> &rhs)
    {
        baseClass::operator = (rhs);
        return *this;
    }

    template <class Y, typename mem_mgr2> strong_ptr& operator=(const weak_ptr<Y, mem_mgr2> &rhs)
    {
        baseClass::operator = (rhs);
        return *this;
    }
};


template <class X, typename mem_mgr=cpp_mem_mgr<X> > class weak_ptr : public base_ptr<X, false, mem_mgr>
{
    typedef base_ptr<X, false, mem_mgr> baseClass;
public:
    // construct empty weak_ptr object
    weak_ptr()
    {
    }

    // construct weak_ptr object for resource owned by rhs
    template<class Y, typename mem_mgr2> weak_ptr(const strong_ptr<Y, mem_mgr2> &rhs) : baseClass(rhs)
    {
    }

    // construct weak_ptr object for resource pointed to by rhs
    weak_ptr(const weak_ptr &rhs) : baseClass(rhs)
    {
    }

    // construct weak_ptr object for resource pointed to by rhs
    template<class Y, typename mem_mgr2> weak_ptr(const weak_ptr<Y, mem_mgr2> &rhs) : baseClass(rhs)
    {
    }

    ~weak_ptr()
    {
    }

    weak_ptr& operator=(const weak_ptr &rhs)
    {
        baseClass::operator =(rhs);
        return *this;
    }

    template <class Y, typename mem_mgr2> weak_ptr& operator=(const weak_ptr<Y, mem_mgr2> &rhs)
    {
        baseClass::operator = (rhs);
        return *this;
    }

    template <class Y, typename mem_mgr2> weak_ptr& operator=(const strong_ptr<Y, mem_mgr2> &rhs)
    {
        baseClass::operator = (rhs);
        return *this;
    }

    // return true if resource no longer exists
    bool expired() const
    {
        return m_counter ? m_counter->expired() : true;
    }

    // convert to strong_ptr
    strong_ptr<X, mem_mgr> lock() const
    {
        return strong_ptr<X, mem_mgr>(*this);
    }

private:
    operator X*()   const throw();
    X& operator*()  const throw();
    X* operator->() const throw();
    X* get()        const throw();
};


//////////////////////////////////////////////////////////////////////////
//
//   function make_strong_ptr
//

template <typename T>
strong_ptr<T, cpp_mem_mgr<T> > make_strong_ptr(void)
{
    return strong_ptr<T, cpp_mem_mgr> ( cpp_mem_mgr<T>::allocate() );
}

template <typename T, typename A1>
strong_ptr<T, cpp_mem_mgr<T> > make_strong_ptr(A1 const &a1)
{
    return strong_ptr<T, cpp_mem_mgr<T> > ( cpp_mem_mgr<T>::allocate(a1) );
}

template <typename T, typename A1, typename A2>
strong_ptr<T, cpp_mem_mgr<T> > make_strong_ptr(A1 const &a1, A2 const &a2)
{
    return strong_ptr<T, cpp_mem_mgr<T> > ( cpp_mem_mgr<T>::allocate(a1, a2) );
}

template <typename T, typename A1, typename A2, typename A3>
strong_ptr<T, cpp_mem_mgr<T> > make_strong_ptr(A1 const &a1, A2 const &a2, A3 const &a3)
{
    return strong_ptr<T, cpp_mem_mgr<T> > ( cpp_mem_mgr<T>::allocate(a1, a2, a3) );
}

template <typename T, typename A1, typename A2, typename A3, typename A4>
strong_ptr<T, cpp_mem_mgr<T> > make_strong_ptr(A1 const &a1, A2 const &a2, A3 const &a3, A4 const &a4)
{
    return strong_ptr<T, cpp_mem_mgr<T> > ( cpp_mem_mgr<T>::allocate(a1, a2, a3, a4) );
}

template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
strong_ptr<T, cpp_mem_mgr<T> > make_strong_ptr(A1 const &a1, A2 const &a2, A3 const &a3, A4 const &a4, A5 const &a5)
{
    return strong_ptr<T, cpp_mem_mgr<T> > ( cpp_mem_mgr<T>::allocate(a1, a2, a3, a4, a5) );
}

template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
strong_ptr<T, cpp_mem_mgr<T> > make_strong_ptr(A1 const &a1, A2 const &a2, A3 const &a3, A4 const &a4, A5 const &a5, A6 const &a6)
{
    return strong_ptr<T, cpp_mem_mgr<T> > ( cpp_mem_mgr<T>::allocate(a1, a2, a3, a4, a5, a6) );
}


template<typename X>
class cpp_arr_mem_mgr {
public:
    static void deallocate(X *p) { delete []p; }
    static X * allocate(int n) { return new X[n]; }
};

template <class X, typename mem_mgr=cpp_arr_mem_mgr<X> > class strong_array : public base_ptr<X, true, mem_mgr>
{
    typedef typename base_ptr<X, true, mem_mgr> baseClass;
public:
    explicit strong_array(X* p = 0) : baseClass(p)
    {
    }

    strong_array(const strong_array& rhs) : baseClass(rhs)
    {
    }

    template<class Y> strong_array(const strong_array<Y, mem_mgr> &rhs) : baseClass(rhs)
    {
    }

    ~strong_array()
    {
    }

    const X & operator[](int i) const
    {
        return get()[i];
    }

    X & operator[](int i)
    {
        return get()[i];
    }

    strong_array& operator=(const strong_array &rhs)
    {
        baseClass::operator = (rhs);
        return *this;
    }

    template <class Y> strong_array& operator=(const strong_array<Y, mem_mgr> &rhs)
    {
        baseClass::operator = (rhs);
        return *this;
    }
private:
    X& operator*()  const throw();
    X* operator->() const throw();
};


}; // namespace smart_ptr


#endif // __SMART_PTR_H__
