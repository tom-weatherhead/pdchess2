/*SDOC************************************************************************

	Module: AutoPtr.h

	Author:	Tom Weatherhead

	Description: Contains an auto_ptr-like implementation; an auto_ptr behaves
	like a regular pointer, except that the object to which it points is destroyed
	when the auto_ptr is destroyed.  This implementation uses a reference counting
	system, so that several auto_ptr objects can refer to the same dynamically
	allocated resource, which is destroyed when there are no longer any auto_ptr
	objects referring to it.

************************************************************************EDOC*/

/*SDOC************************************************************************

  Revision Record

	Rev		Date				Auth	Changes
	===		====				====	=======
		0		2002/03/17	TAW		Created.

************************************************************************EDOC*/


#ifndef _AUTO_PTR_H_
#define _AUTO_PTR_H_

#include "exception.h"


// auto_ptr example: Stroustrup, Special Edition, p. 368.

// TAW 2002/03/16 : New version of CAutoPtr which uses CAutoPtrRefs.


// TAW 2002/03/26 : Allow ptr to create a new copy of the referred object.
#define AUTOPTR_SUPPORT_SPLIT		1


// ************************************
// **** class template CAutoPtrRef ****
// ************************************


template<class X> class CAutoPtrRef
{
private:
	X * m_ptr;								// This member could be const.
	int m_nReferenceCount;		// This member could be unsigned.

	// Private copy constructor; ie. disallow copy construction.

	inline CAutoPtrRef(
		const CAutoPtrRef & Src
		) throw( CException )
		: m_ptr( 0 ),
			m_nReferenceCount( 0 )
	{
		ThrowException( eStatus_NotImplemented );
		// Because of the exception, the destructor won't be run,
		// so the zero reference count isn't a problem.
	}

	// Private assignment operator; ie. disallow assignment.

	inline CAutoPtrRef & operator=(
		const CAutoPtrRef & Src
		) throw( CException )
	{

		if( &Src != this )
		{
			// Don't modify this object; just throw the exception.
			ThrowException( eStatus_NotImplemented );
		}

		return( *this );
	}

public:
	//typedef X element_type;

	// Main (default) constructor.

	explicit inline CAutoPtrRef(
		X * ptr = 0
		) throw()
		: m_ptr( ptr ),
			m_nReferenceCount( 1 )
	{
	}

	// Destructor.

	virtual ~CAutoPtrRef( void ) throw()
	{
		//AssertCS( m_pClientServices, m_nReferenceCount == 0 );
		delete m_ptr;	// TAW 2002/03/16 : Should have no effect on a zero pointer.
		m_ptr = 0;
	}

	// Accessor functions.

	inline X * GetPtr( void ) const throw()
	{
		return( m_ptr );
	}

	inline void IncrementReferenceCount( void ) throw()
	{
		//AssertCS( m_pClientServices, m_nReferenceCount > 0 );
		m_nReferenceCount++;
	}

	inline bool DecrementReferenceCount( void ) throw()
	{
		//AssertCS( m_pClientServices, m_nReferenceCount > 0 );
		// Destroy this object if and only if true is returned here.
		return( ( --m_nReferenceCount <= 0 ) ? true : false );
	}

	inline bool ReferenceCountIsOne( void ) throw()
	{
		//AssertCS( m_pClientServices, m_nReferenceCount > 0 );
		// TAW 2002/03/27 : Use "<= 1" instead of "== 1" for safety (Paranoid).
		return( ( m_nReferenceCount <= 1 ) ? true : false );
	}
}; // CAutoPtrRef


// **************************************
// **** class template CBasicAutoPtr ****
// **************************************


template<class X> class CBasicAutoPtr
{
protected:
	CAutoPtrRef<X> * m_pRef;

	inline X * GetPtrFromRef( void ) const throw()
	{
		return( ( m_pRef != 0 ) ? m_pRef->GetPtr() : 0 );
	}

	void Reset( X * p = 0 ) throw( CException )
	{

		if( m_pRef != 0 )
		{

			if( p == GetPtrFromRef() )
			{
				return;	// We're already in the correct state.
			}

			if( m_pRef->DecrementReferenceCount() )
			{
				delete m_pRef;
			}

			// To prevent double-freeing the old pointer in case the following CAutoPtrRef construction fails.
			// We want to set m_pRef to zero even if we haven't deleted the object to which it points;
			// we simply no longer refer to it.
			m_pRef = 0;
		}

		if( p != 0 )
		{

			try
			{
				// CAutoPtrRef<X> constructor won't throw any exceptions, but the "new" might.
				m_pRef = new CAutoPtrRef<X>( p );
			}
			catch( ... )
			{
			}

			if( m_pRef == 0 )
			{
				//SignalCS( m_pClientServices );
				ThrowException( eStatus_ResourceAcquisitionFailed );
			}
		}
	} // Reset()

public:
	//typedef X element_type;

	// Main (default) constructor.

	inline CBasicAutoPtr( X * p = 0 ) throw( CException )
		: m_pRef( 0 )
	{
		Reset( p );
	} // Main (default) constructor.

	// Public copy constructor.

	CBasicAutoPtr( const CBasicAutoPtr & Src ) throw()
		: m_pRef( Src.m_pRef )
	{

		if( m_pRef != 0 )
		{
			m_pRef->IncrementReferenceCount();
		}
	} // Copy constructor.

	// Destructor.

	virtual ~CBasicAutoPtr( void ) throw()
	{

		if( m_pRef != 0  &&  m_pRef->DecrementReferenceCount() )
		{
			delete m_pRef;
		}

		m_pRef = 0;
	} // Destructor.

	// Public assignment operator.

	inline CBasicAutoPtr & operator=( const CBasicAutoPtr & Src ) throw()
	{

		if( &Src != this )
		{
			//m_pClientServices = Src.m_pClientServices;

			if( m_pRef != 0  &&  m_pRef->DecrementReferenceCount() )
			{
				delete m_pRef;
			}

			m_pRef = Src.m_pRef;

			if( m_pRef != 0 )
			{
				m_pRef->IncrementReferenceCount();
			}
		}

		return( *this );
	} // Assignment operator.

	// Other functions.

	// Implicit cast of CBasicAutoPtr<X> to X *.

	operator X *( void ) const throw()
	{
		return( GetPtrFromRef() );
	}

	X & operator*() const throw( CException )
	{
		X * ptr = GetPtrFromRef();

		if( ptr == 0 )
		{
			//SignalCS( m_pClientServices );
			ThrowException( eStatus_InternalError );
		}

		return *ptr;
	}

	// TAW 2002/03/09 : I added these 3, to make CAutoPtr behave more like a real pointer.

	inline bool operator==( X * p ) const throw()
	{
		return( p == GetPtrFromRef() );
	}

	inline bool operator!=( X * p ) const throw()
	{
		return( p != GetPtrFromRef() );
	}

	inline CBasicAutoPtr & operator=( X * p ) throw( CException )
	{
		Reset( p );
		return( *this );
	}

#if 0
	inline bool IsNull( void ) const throw()
	{
		return( GetPtrFromRef() == 0 );
	}
#endif

#ifdef AUTOPTR_SUPPORT_SPLIT
	void EnsurePrivateCopy( void ) throw( CException )
	{
		X * pOld = GetPtrFromRef();
		X * pNew = 0;

		if( pOld == 0  ||  m_pRef == 0  ||  m_pRef->ReferenceCountIsOne() )
		{
			// Nothing to do; either there is no object, or
			// this AutoPtr is already the sole owner of the object.
			return;
		}

		try
		{
			// X's copy constructor is used here.
			pNew = new X( *pOld );
		}
		catch( const CException & )
		{
			throw;
		}
		catch( ... )
		{
			// Do nothing; pNew is already zero, and we'll throw an exception below.
		}

		if( pNew == 0 )
		{
			//SignalCS( m_pClientServices );
			ThrowException( eStatus_ResourceAcquisitionFailed );
		}

		Reset( pNew );
	}
#endif
}; // CBasicAutoPtr


// *********************************
// **** template class CAutoPtr ****
// *********************************


template<class X> class CAutoPtr : public CBasicAutoPtr<X>
{
public:

	// Main (default) constructor.

	/* explicit */ inline CAutoPtr(
		X * p = 0
		) throw( CException )
		: CBasicAutoPtr<X>( p )
	{
	} // Main (default) constructor.

#if 1	// TAW 2002/04/23.
	// Construct from an integer (ie. zero; the zero pointer).

	inline CAutoPtr(
		int
		) throw( CException )
		: CBasicAutoPtr<X>( 0, 0 )
	{
	}
#endif

	// Public copy constructor.

	CAutoPtr( const CAutoPtr & Src ) throw()
		: CBasicAutoPtr<X>( Src )
	{
	} // Copy constructor.

	// Destructor.

	virtual ~CAutoPtr( void ) throw()
	{
	} // Destructor.

	// Public assignment operator.

	inline CAutoPtr & operator=( const CAutoPtr & Src ) throw()
	{

		if( &Src != this )
		{
			CBasicAutoPtr<X>::operator=( Src );
		}

		return( *this );
	} // Assignment operator.

	// Other (non-basic) functions, which cannot be applied to atomic types.

	X * operator->() const throw( CException )
	{
		X * ptr = GetPtrFromRef();

		if( ptr == 0 )
		{
			//SignalCS( m_pClientServices );
			ThrowException( eStatus_InternalError );
		}

		return ptr;
	} // operator->()

	// TAW 2002/03/09 : I added these 3, to make CAutoPtr behave more like a real pointer.

#if 0
	inline bool operator==( X * p ) const throw()
	{
		return( p == GetPtrFromRef() );
	}

	inline bool operator!=( X * p ) const throw()
	{
		return( p != GetPtrFromRef() );
	}
#endif

	inline CAutoPtr & operator=( X * p ) throw( CException )
	{
		Reset( p );
		return( *this );
	}
}; // CAutoPtr


#endif	//#ifndef _AUTO_PTR_H_


// **** End of File ****
