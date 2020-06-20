/*SDOC************************************************************************

	Module: Exception.h

	Author:	Tom Weatherhead

	Description: Contains an exception class.

************************************************************************EDOC*/

/*SDOC************************************************************************

  Revision Record

	Rev		Date				Auth	Changes
	===		====				====	=======
		0		2002/02/15	TAW		Created.

************************************************************************EDOC*/


#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_


/*
**
**	 Type Definitions
**
*/


typedef enum
{
	eStatus_Success = 0,													// No error.
	eStatus_UserAbort,														// The client requested an abort.
	eStatus_UnrecognizedException,								// Return this from "catch( ... ) { }".
	eStatus_NotImplemented,												// A client didn't implement a service.
	eStatus_InvalidParameter,											// Bad parameter passed to a function.
	eStatus_InternalError,												// An error in our algorithm.  Be more specific, if possible.
	eStatus_ResourceAcquisitionFailed,						// eg. "new" returned zero.
	eStatus_ConstructorFailed,										// A general fatal error in a constructor.
	eStatus_ClientServiceError,										// A user-supplied client service caused an error.
	eStatus_ClientServiceException,								// A user-supplied client service threw an exception.
	eStatus_IllegalOperation
	//eStatus_GeneralFailure,											// Vague.  Use when there isn't a more specific message available.
	//eStatus_NoClientServices,										// Impossible.
}
StatusType;


class CException
{
private:

	const char * m_pcFile;
	int m_nLine;
	StatusType m_eStatus;

	// Private assignment operator.

	inline CException & operator=( const CException & Src ) throw()
	{

		if( &Src != this )
		{
			m_pcFile = Src.m_pcFile;
			m_nLine = Src.m_nLine;
			m_eStatus = Src.m_eStatus;
		}

		return( *this );
	}

public:

	// Main constructor.

	inline CException(
		const char * pcFile,
		int nLine,
		StatusType eStatus ) throw()
	{
		m_pcFile = pcFile;
		m_nLine = nLine;
		m_eStatus = eStatus;
	}

	// Public copy constructor; used when an object of this type is thrown as an exception (Stroustrup reference?)

	inline CException( const CException & Src ) throw()
		: m_pcFile( Src.m_pcFile ),
			m_nLine( Src.m_nLine ),
			m_eStatus( Src.m_eStatus )
	{
	}

	// Accessors.

	inline const char * GetFilename( void ) const throw()
	{
		return( m_pcFile );
	}

	inline int GetLineNumber( void ) const throw()
	{
		return( m_nLine );
	}

	inline StatusType GetStatusCode( void ) const throw()
	{
		return( m_eStatus );
	}
}; // class CException


/*
**
**	 Preprocessor Definitions (Stroustrup: use as few of these as possible)
**
*/


#define ThrowException(s)	throw( CException( __FILE__, __LINE__, (s) ) )


#endif	//#ifndef _EXCEPTION_H_


// **** End of File ****
