// pdchess2 - Tom Weatherhead - June 8, 2002

// To Do:
// - Use the GeneratedMoveType and the > and >= operators.

#include <iostream>
#include <cstdlib>			// For srand(), rand().
#include <ctime>			// For time().
#include <vector>

#include "auto-ptr.h"

// TODO: Use "using" to use only the parts of std that are actually used.
using namespace std;

//#define TAW_DEBUG		1

static const int cnBoardSize = 8;
static const int cnBoardArea = cnBoardSize * cnBoardSize;


enum GeneratedMoveType
{
	eGenMoveType_All = 0,
	eGenMoveType_Attacking,	// A move that could capture an opposing piece on the dest. square, if there was such a piece there.  Excludes castling and straight-forward pawn movement.
	eGenMoveType_Capturing	// A move that does capture an opposing piece.
};


#if 1
static inline void Assert( bool b )
{

	if( !b )
	{
		__asm int 3
	}
}
#elif 1
#define Assert( b ) { if( !(b) ) { __asm int 3 } }
#else
#define Assert( b ) (void)0
#endif


// **** Class C2DVector ****

class C2DVector
{
public:
	int m_nDX;
	int m_nDY;

	C2DVector( int nDX, int nDY );

	C2DVector( const C2DVector & Src );

	C2DVector & operator=( const C2DVector & Src );

	bool operator==( const C2DVector & Src ) const;

	inline int GetDX( void ) const { return( m_nDX ); }

	inline int GetDY( void ) const { return( m_nDY ); }
};


C2DVector::C2DVector( int nDX, int nDY )
	: m_nDX( nDX ),
		m_nDY( nDY )
{
}


C2DVector::C2DVector( const C2DVector & Src )
	: m_nDX( Src.m_nDX ),
		m_nDY( Src.m_nDY )
{
}


C2DVector & C2DVector::operator=( const C2DVector & Src )
{

	if( this != &Src )
	{
		m_nDX = Src.m_nDX;
		m_nDY = Src.m_nDY;
	}

	return( *this );
}


bool C2DVector::operator==( const C2DVector & Src ) const
{
	return( m_nDX == Src.m_nDX  &&  m_nDY == Src.m_nDY );
}


enum PieceTypeType
{
	ePieceType_King = 0,
	ePieceType_Queen,
	ePieceType_Rook,
	ePieceType_Bishop,
	ePieceType_Knight,
	ePieceType_Pawn,
	ePieceType_Null,
	eNumPieceTypes = ePieceType_Null
};


class CMove
{
	// Move Table == an array of 7 list<CMove>
public:
	int m_nSrcSquare;	// == 8 * row + col
	int m_nDstSquare;
	PieceTypeType m_PromotedTo;
}; // class CMove


// **** Class CPieceArchetype ****

class CPieceArchetype
{
private:
	void AddAllOrientations( const C2DVector & Dir );

public:
	PieceTypeType m_PieceType;
	char m_Printable;		// Printable representation (upper case).
	double m_dValue;
	bool m_bUnlimitedRange;	// true for Bishop, Rook, Queen.
	vector<C2DVector> m_Directions;

	CPieceArchetype( PieceTypeType PieceType, int nOwner );
}; // class CPieceArchetype


CPieceArchetype::CPieceArchetype( PieceTypeType PieceType )
	: m_PieceType( PieceType ),
		m_Printable( '?' ),
		m_dValue( 0 ),
		m_bUnlimitedRange( false )
{
	static const C2DVector StraightVector( 1, 0 );
	static const C2DVector DiagonalVector( 1, 1 );
	static const C2DVector KnightVector( 2, 1 );

	switch( m_PieceType )
	{
		case ePieceType_King:
			m_Printable = 'K';
			m_dValue = 1000.0;
			m_bUnlimitedRange = false;
			AddAllOrientations( StraightVector );
			AddAllOrientations( DiagonalVector );
			break;

		case ePieceType_Queen:
			m_Printable = 'Q';
			m_dValue = 9.0;
			m_bUnlimitedRange = true;
			AddAllOrientations( StraightVector );
			AddAllOrientations( DiagonalVector );
			break;

		case ePieceType_Rook:
			m_Printable = 'R';
			m_dValue = 5.0;
			m_bUnlimitedRange = true;
			AddAllOrientations( StraightVector );
			break;

		case ePieceType_Bishop:
			m_Printable = 'B';
			m_dValue = 3.125;
			m_bUnlimitedRange = true;
			AddAllOrientations( DiagonalVector );
			break;

		case ePieceType_Knight:
			m_Printable = 'N';
			m_dValue = 3.0;
			m_bUnlimitedRange = false;
			AddAllOrientations( KnightVector );
			break;

		case ePieceType_Pawn:
			m_Printable = 'P';
			m_dValue = 1.0;
			m_bUnlimitedRange = false;
			// Leave Direction vector blank.
			break;

		default:
			ThrowException( eStatus_InternalError );
	}
} // CPieceArchetype::CPieceArchetype()


template <class C>
bool IsMemberOfVector( const C & obj, const vector<C> & vec )
{
	const vector<C>::size_type knVecSize = vec.size();

	for( vector<C>::size_type i = 0; i < knVecSize; ++i )
	{

		if( vec[i] == obj )
		{
			return( true );
		}
	}

	return( false );
}


void CPieceArchetype::AddAllOrientations( const C2DVector & Dir )
{

	if( IsMemberOfVector( Dir, m_Directions ) )
	{
		return;	// The seed vector is already in the vector vector.
	}

	vector<C2DVector>::size_type i = 0;
	const vector<C2DVector>::size_type knVectorStart = m_Directions.size();
	vector<C2DVector>::size_type nVectorEnd = 0;

	m_Directions.push_back( Dir );

	// 1) Reflect in the X axis.
	nVectorEnd = m_Directions.size();

	for( i = knVectorStart; i < nVectorEnd; ++i )
	{
		const C2DVector NewDir( m_Directions[i].GetDX(), - m_Directions[i].GetDY() );

		if( !IsMemberOfVector( NewDir, m_Directions ) )
		{
			m_Directions.push_back( NewDir );
		}
	}

	// 2) Reflect in the Y axis.
	nVectorEnd = m_Directions.size();

	for( i = knVectorStart; i < nVectorEnd; ++i )
	{
		const C2DVector NewDir( - m_Directions[i].GetDX(), m_Directions[i].GetDY() );

		if( !IsMemberOfVector( NewDir, m_Directions ) )
		{
			m_Directions.push_back( NewDir );
		}
	}

	// 3) Reflect in the line X == Y.
	nVectorEnd = m_Directions.size();

	for( i = knVectorStart; i < nVectorEnd; ++i )
	{
		const C2DVector NewDir( m_Directions[i].GetDY(), m_Directions[i].GetDX() );

		if( !IsMemberOfVector( NewDir, m_Directions ) )
		{
			m_Directions.push_back( NewDir );
		}
	}
} // CPieceArchetype::AddAllOrientations()


// **** Class CPiece ****

class CPlayer;

class CPiece
{
public:
	CPieceArchetype * m_pArchetype;
	CPlayer & m_Owner; //int m_nOwner;				// 0 for White, 1 for Black.
	int m_nRow;
	int m_nCol;
	bool m_bCaptured;
}; // class CPiece


// **** Class CPlayer ****

class CGame;

class CPlayer
{
private:
	int SquareToPieceTypeIndex( const CPiece * pSquare ) const; // Could be static.

public:
	const int m_knSelfID;				// 0 for White, 1 for Black.
	CGame & m_Game;
	CPlayer & m_Opponent;
	bool m_bCanCastleKingside;
	bool m_bCanCastleQueenside;
	vector<CPiece> m_Pieces;

	CPlayer( int nSelfID, CGame & game, CPlayer & opponent );
	void CreatePieces( void );
	double TotalMaterialValue( void ) const;
	double FindBestMove( CMove * pBestMove, int nMaxPly,
		bool bDoAlphaBetaPruning, double dParent, double dBestUncleLine );
};


CPlayer::CPlayer( int nSelfID, CGame & game, CPlayer & opponent )
	: m_knSelfID( nSelfID ),
		m_Game( game ),
		m_Opponent( opponent ),
		m_bCanCastleKingside( true ),
		m_bCanCastleQueenside( true )
{
	CreatePieces();
}


double CPlayer::TotalMaterialValue( void ) const
{
	return( 0.0 );	// TAW_TODO.
}

int CPlayer::SquareToPieceTypeIndex( const CPiece * pSquare ) const
{

	if( pSquare == 0 )
	{
		// There is no piece on this square.
		return( 6 );
	}

	switch( pSquare->m_pArchetype->m_PieceType )
	{
		case ePieceType_King:
			return( 0 );

		case ePieceType_Queen:
			return( 1 );

		case ePieceType_Rook:
			return( 2 );

		case ePieceType_Bishop:
			return( 3 );

		case ePieceType_Knight:
			return( 4 );

		case ePieceType_Pawn:
			return( 5 );

		default:
			// Error.
			break;
	}

	return( 6 );
}


// **** Class CGame ****

class CGame
{
private:
	// King, Queen, Rook, Bishop, Knight, Pawn.
	const CPieceArchetype m_KingArchetype;
	const CPieceArchetype m_QueenArchetype;
	const CPieceArchetype m_RookArchetype;
	const CPieceArchetype m_BishopArchetype;
	const CPieceArchetype m_KnightArchetype;
	const CPieceArchetype m_PawnArchetype;
	CPiece * m_aBoard[cnBoardArea];

	CPlayer m_WhitePlayer;
	CPlayer m_BlackPlayer;

	int m_nPawnCapturableViaEnPassant;

	void InitializeBoard( void );
	void PrintBoard( void ) const;

public:

	CGame( void );

	void Play( void ) throw( CException );

}; // class CGame


void CPlayer::CreatePieces( void )
{
	const int knBackRow = 7 * m_knSelfID;
	const int knFrontRow = 5 * m_knSelfID + 1;

	m_Pieces.push_back( CPiece( m_kGame.m_RookArchetype, *this, knBackRow, 0 ) );
	m_Pieces.push_back( CPiece( m_kGame.m_KnightArchetype, *this, knBackRow, 1 ) );
	m_Pieces.push_back( CPiece( m_kGame.m_BishopArchetype, *this, knBackRow, 2 ) );
	m_Pieces.push_back( CPiece( m_kGame.m_QueenArchetype, *this, knBackRow, 3 ) );
	m_Pieces.push_back( CPiece( m_kGame.m_KingArchetype, *this, knBackRow, 4 ) );
	m_Pieces.push_back( CPiece( m_kGame.m_BishopArchetype, *this, knBackRow, 5 ) );
	m_Pieces.push_back( CPiece( m_kGame.m_KnightArchetype, *this, knBackRow, 6 ) );
	m_Pieces.push_back( CPiece( m_kGame.m_RookArchetype, *this, knBackRow, 7 ) );

	for( int i = 0; i < 8; ++i )
	{
		m_Pieces.push_back( CPiece( m_kGame.m_PawnArchetype, *this, knFrontRow, i ) );
	}
}


void CPlayer::GenerateMoves( vector<CMove> & generatedMoves, bool bGenerateAttackingMovesOnly )
{
	// Generate the vector of all possible legal moves, including castling.
	// The moves are sorted by the value of the captured piece, if any;
	// King captures come first, since they end the game.
	vector<CMove> GeneratedMovesAList[7];
	const int knBackRow = 7 * m_knSelfID;
	const int knPawnStartRow = 5 * m_knSelfID + 1;
	const int knPawnPromotionRow = 7 * ( 1 - m_knSelfID );
	const int knPawnRowVector = 1 - 2 * m_knSelfID;
	const int knNumPieces = m_Pieces.size();
	int i = 0;

	for( i = 0; i < knNumPieces; ++i )
	{
		const CPiece * const kpPiece = &m_Pieces[i];
		const int knSrcIndex = kpPiece->m_nRow * 8 + kpPiece->m_nCol;

		if( kpPiece->m_pArchetype->m_PieceType == ePieceType_Pawn )
		{
			// Generate all possible legal pawn moves.
			const int knSrcRow = kpPiece->m_nRow;
			const int knSrcCol = kpPiece->m_nCol;
			int nDstRow = 0;
			int nDstCol = 0;
			const CPiece * pDstSquare = 0;

			// For pawns, be aware of:
			// 1) 1- or 2-square initial move ahead;
			// 2) Move forward, capture diagonally;
			// 3) Capturing en passant;
			// 4) Pawn promotion to knight, bishop, rook, or queen.

			if( !bGenerateAttackingMovesOnly )
			{
				// Try to move the pawn ahead one square.
				nDstRow = knSrcRow + knPawnRowVector;
				nDstCol = knSrcCol;

				if( nDstRow >= 0  &&  nDstRow < 8 )
				{
					pDstSquare = m_Game.m_aBoard[nDstRow * 8 + nDstCol];

					if( pDstSquare == 0 )
					{
						// Move the pawn ahead one square.

						if( nDstRow == knPawnPromotionRow )
						{
							// Promote the pawn (without capture).
							GeneratedMovesAList[6].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Queen ) );
							GeneratedMovesAList[6].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Rook ) );
							GeneratedMovesAList[6].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Bishop ) );
							GeneratedMovesAList[6].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Knight ) );
						}
						else
						{
							GeneratedMovesAList[6].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Null ) );
						}

						// Try to move the pawn ahead two squares if it's the pawn's first move.
						nDstRow = knSrcRow + 2 * knPawnRowVector;

						// Some of these conditions are redundant.
						if( knSrcRow == knPawnStartRow /* &&  nDstRow >= 0  &&  nDstRow < 8 */ )
						{
							pDstSquare = m_Game.m_aBoard[nDstRow * 8 + nDstCol];

							if( pDstSquare == 0 )
							{
								// Move the pawn ahead two squares.
								// Pawn promotion is impossible here.
								GeneratedMovesAList[6].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Null ) );
							}
						}
					}
				}
			}

			// Try to attack diagonally.
			static const int kanDX[2] = { -1, 1 };

			for( int j = 0; j < 2; ++j )
			{
				nDstRow = knSrcRow + knPawnRowVector;
				nDstCol = knSrcCol + kanDX[j];

				if( nDstRow >= 0  &&  nDstRow < 8  &&
						nDstCol >= 0  &&  nDstCol < 8 )
				{
					pDstSquare = m_Game.m_aBoard[nDstRow * 8 + nDstCol];

					if( pDstSquare != 0  &&  &pDstSquare->m_Owner != this )
					{
						// Attack diagonally and capture the piece on the destination square.
						const int knPieceTypeIndex = SquareToPieceTypeIndex( pDstSquare );

						if( nDstRow == knPawnPromotionRow )
						{
							// Promote the pawn (with capture).
							GeneratedMovesAList[knPieceTypeIndex].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Queen ) );
							GeneratedMovesAList[knPieceTypeIndex].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Rook ) );
							GeneratedMovesAList[knPieceTypeIndex].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Bishop ) );
							GeneratedMovesAList[knPieceTypeIndex].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Knight ) );
						}
						else
						{
							GeneratedMovesAList[knPieceTypeIndex].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Null ) );
						}
					}
				}
			}

			// Try to capture en passant.

			if( m_Game.m_nPawnCapturableViaEnPassant >= 0  &&  m_Game.m_nPawnCapturableViaEnPassant < 64 )
			{
				const int knCapturablePawnRow = m_Game.m_nPawnCapturableViaEnPassant % 8;
				const int knCapturablePawnCol = m_Game.m_nPawnCapturableViaEnPassant / 8;

				// Assert( knCapturablePawnRow == 5 - m_knSelfID );

				if( knCapturablePawnRow == knSrcRow  &&
						abs( knCapturablePawnCol - knSrcCol ) == 1 )
				{
					nDstRow = knSrcRow + knPawnRowVector;
					nDstCol = knCapturablePawnCol;

					// Index 5: A pawn is capturing another pawn.  No promotion.
					GeneratedMovesAList[5].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Null ) );
				}
			}
		}
		else
		{
			// Use the piece's vector of direction vectors.
			const vector<C2DVector> & directions = kpPiece->m_pArchetype->m_Directions;
			const int knNumDirections = directions.size();

			for( int j = 0; j < knNumDirections; ++j )
			{
				const C2DVector & CurrentDirection = direction[j];
				int nDstRow = kpPiece->m_nRow;
				int nDstCol = kpPiece->m_nCol;

				do
				{
					nDstRow += CurrentDirection.GetDY();
					nDstCol += CurrentDirection.GetDX();

					if( nDstRow < 0  ||  nDstRow >= 8  ||
							nDstCol < 0  ||  nDstCol >= 8 )
					{
						// We've moved off the board.
						break;
					}

					const CPiece * const kpDstSquare = m_Game.m_aBoard[nDstRow * 8 + nDstCol];

					if( kpDstSquare != 0  &&  &kpDstSquare->m_Owner == this )
					{
						// We've bumped into another one of our own pieces.
						break;
					}

					// We have a legal move!  Add it to the table.
					GeneratedMovesAList[SquareToPieceTypeIndex( kpDstSquare )].push_back( CMove( knSrcIndex, nDstRow * 8 + nDstCol, ePieceType_Null ) );

					if( kpDstSquare != 0 )
					{
						// The move we just generated was a capture; we can go no further.
						break;
					}
				}
				while( kpPiece->m_pArchetype->m_bUnlimitedRange );

			}
		}
	}

	if( !bGenerateAttackingMovesOnly )
	{
		// Try to generate the castling moves:
		// 1) The king and the rook must not have been moved yet;
		// 2) The squares between the king and the rook must be empty;
		// 3) The squares that the king moves through and to must not be under attack;
		// 4) The king must not be in check (you can't castle to escape check).

		vector<CMove> opponentsAttackingMoves;
		bool bOpponentsAttackingMovesGenerated = false;

		if( m_bCanCastleKingside  &&	// King and kingside rook not moved yet.
				m_Game.m_aBoard[knBackRow * 8 + 5] == 0  &&	// f1 is vacant.
				m_Game.m_aBoard[knBackRow * 8 + 6] == 0 )		// g1 is vacant.
		{
			// Generate attacking moves only.
			m_Opponent.GenerateMoves( opponentsAttackingMoves, true );
			bOpponentsAttackingMovesGenerated = true;

			if(	!m_Opponent.IsAttackingSquare( opponentsAttackingMoves, knBackRow, 4 )  &&	// e1 is not under attack (ie. the king is not in check).
					!m_Opponent.IsAttackingSquare( opponentsAttackingMoves, knBackRow, 5 )  &&	// f1 is not under attack.
					!m_Opponent.IsAttackingSquare( opponentsAttackingMoves, knBackRow, 6 ) )		// g1 is not under attack.
			{
				// Board index 64 means castle kingside.
				GeneratedMovesAList[6].push_back( CMove( 64, 64, ePieceType_Null ) );
			}
		}

		if( m_bCanCastleQueenside  &&	// King and queenside rook not moved yet.
				m_Game.m_aBoard[knBackRow * 8 + 1] == 0  &&	// b1 is vacant.
				m_Game.m_aBoard[knBackRow * 8 + 2] == 0  &&	// c1 is vacant.
				m_Game.m_aBoard[knBackRow * 8 + 3] == 0 )		// d1 is vacant.
		{

			if( !bOpponentsAttackingMovesGenerated )
			{
				// Generate attacking moves only.
				m_Opponent.GenerateMoves( opponentsAttackingMoves, true );
				bOpponentsAttackingMovesGenerated = true;
			}

			if( !m_Opponent.IsAttackingSquare( opponentsAttackingMoves, knBackRow, 2 )  &&	// c1 is not under attack.
					!m_Opponent.IsAttackingSquare( opponentsAttackingMoves, knBackRow, 3 )  &&	// d1 is not under attack.
					!m_Opponent.IsAttackingSquare( opponentsAttackingMoves, knBackRow, 4 ) )		// e1 is not under attack (ie. the king is not in check).
			{
				// Board index 65 means castle queenside.
				GeneratedMovesAList[6].push_back( CMove( 65, 65, ePieceType_Null ) );
			}
		}
	}

	// Collate all generated moves into a single vector, sorted by captured piece value.

	generatedMoves.clear();

	for( i = 0; i < 7; ++i )
	{
		generatedMoves.insert( generatedMoves.end(), GeneratedMovesAList[i].begin(), GeneratedMovesAList[i].end() );
	}
}


double CPlayer::FindBestMove( CMove * pBestMove, int nMaxPly,
	bool bDoAlphaBetaPruning, double dParentMoveValue, double dBestUncleLineValue )
{
	vector<CMove> generatedMoves;
	vector<CMove> bestMoves;

	// Generate all moves, including non-attacking moves.
	GenerateMoves( generatedMoves, false );

	// Try each move in the vector until:
	// 1) The game ends due to king capture or draw;
	// 2) Alpha-Beta pruning terminates the search.
	const int knNumGeneratedMoves = generatedMoves.size();
	double dBestLineValue = -2000.0;
	int i = 0;

	for( i = 0; i < knNumGeneratedMoves; ++i )
	{
		// Make the given move, but be able to undo it.
		const CMove & currentMove = generatedMoves[i];
		bool bCastlingMove = false;
		const bool kbOldCanCastleKingside = m_bCanCastleKingside;
		const bool kbOldCanCastleQueenside = m_bCanCastleQueenside;
		const int knOldPawnCapturableViaEnPassant = m_Game.m_nPawnCapturableViaEnPassant;

		m_Game.m_nPawnCapturableViaEnPassant = -1;

		if( currentMove.m_nSrcSquare == 64  ||
				currentMove.m_nSrcSquare == 65 )
		{
			// A castling move.
			Assert( currentMove.m_nDstSquare == currentMove.m_nSrcSquare );
			bCastlingMove = true;

			const int knRow = 7 * m_knSelfID;

			nSrcRow = knRow;
			nSrcCol = 4;
			nDstRow = knRow;
			nSrcRow2 = knRow;
			nDstRow2 = knRow;

			if( currentMove.m_nSrcSquare == 64 )
			{
				// Castle on the kingside.
				nDstCol = 6;
				nSrcCol2 = 7;
				nDstCol2 = 5;
			}
			else
			{
				// Castle on the queenside.
				nDstCol = 2;
				nSrcCol2 = 0;
				nDstCol2 = 3;
			}

			// No capturing can occur here, so we don't need to track any captured pieces.
			ppSrcSquare = &m_Game.m_anBoard[nSrcRow * 8 + nSrcCol];	// King.
			ppDstSquare = &m_Game.m_anBoard[nDstRow * 8 + nDstCol];	// King.
			ppSrcSquare2 = &m_Game.m_anBoard[nSrcRow2 * 8 + nSrcCol2];	// Rook.
			ppDstSquare2 = &m_Game.m_anBoard[nDstRow2 * 8 + nDstCol2];	// Rook.

			// First, Assert that everything is in the right place.
			;

			*ppDstSquare = *ppSrcSquare;
			*ppSrcSquare = 0;
			(*ppDstSquare)->m_nRow = nDstRow;
			(*ppDstSquare)->m_nCol = nDstCol;

			*ppDstSquare2 = *ppSrcSquare2;
			*ppSrcSquare2 = 0;
			(*ppDstSquare2)->m_nRow = nDstRow2;
			(*ppDstSquare2)->m_nCol = nDstCol2;

			// A player can only castle once.
			m_bCanCastleKingside = false;
			m_bCanCastleQueenside = false;
		}
		else
		{
			// A non-castling one-piece move.
			Assert( currentMove.m_nSrcSquare >= 0 );
			Assert( currentMove.m_nSrcSquare < 64 );
			Assert( currentMove.m_nDstSquare >= 0 );
			Assert( currentMove.m_nDstSquare < 64 );

			nSrcRow = currentMove.m_nSrcSquare / 8;
			nSrcCol = currentMove.m_nSrcSquare % 8;
			nDstRow = currentMove.m_nDstSquare / 8;
			nDstCol = currentMove.m_nDstSquare % 8;

			ppSrcSquare = &m_Game.m_aBoard[nSrcRow * 8 + nSrcCol];
			ppDstSquare = &m_Game.m_aBoard[nDstRow * 8 + nDstCol];

			pMovingPiece = *ppSrcSquare;

			// First, Assert that everything is in the right place.
			Assert( pMovingPiece != 0 );

			// Handle en passant captures, where the captured piece isn't on the dest. square.
			nCapturedCol = nDstCol;

			if( pMovingPiece->m_pArchetype->m_PieceType == ePieceType_Pawn  &&
					nDstCol != nSrcCol  &&		// The pawn is capturing something.
					*ppDstSquare == 0 )				// The dest. square is vacant.
			{
				// En passant capture.
				nCapturedRow = nSrcRow;
			}
			else
			{
				nCapturedRow = nDstRow;
			}

			ppCaptureSquare = &m_Game.m_aBoard[nCapturedRow * 8 + nCapturedCol];
			pCapturedPiece = *ppCaptureSquare;

			if( pCapturedPiece != 0 )
			{
				// Assert that the captured piece is an opposing piece, not your own.
				;

				// Mark the captured piece as captured.
				pCapturedPiece->m_bCaptured = true;
			}

			// Update the castling flags, if necessary.
			// If the king moves, both castling flags are set to false.
			// If a rook moves from its original position, that side's castling flag is set to false.
			;

			// Set the PawnCapturableViaEnPassant board index, if necessary.
			;

			// Update the board to reflect the move.
			*ppCaptureSquare = 0;
			*ppDstSquare = *ppSrcSquare;
			*ppSrcSquare = 0;
			(*ppDstSquare)->m_nRow = nDstRow;
			(*ppDstSquare)->m_nCol = nDstCol;
		}

		dLineValue = dCapturedPieceValue;

		// Recurse if we're not too deep, and if the game isn't already over.

		if( nMaxPly > 0  &&  dCapturedPieceValue < 1000.0 )
		{
			dLineValue -= FindBestMove( 0, nMaxPly - 1, i > 0, dCapturedPieceValue, dBestLineValue );
		}

		// Undo the given move:
		// 1) Restore the castling flags.
		// 2) Restore the pawn-capturable-by-en-passant board index.
		// 3) Restore the captured piece, if any.
		// 4) Restore the moved piece(s) to its/their previous position(s).
		m_bCanCastleKingside = kbOldCanCastleKingside;
		m_bCanCastleQueenside = kbOldCanCastleQueenside;
		m_Game.m_nPawnCapturableViaEnPassant = knOldPawnCapturableViaEnPassant;

		// Record the move, if it's a best move.
		const bool kbNewBestLine = dLineValue > dBestLineValue;

		if( pBestMove != 0  &&  dLineValue >= dBestLineValue )
		{

			if( kbNewBestLine )
			{
				dBestLineValue = dLineValue;
				bestMoves.clear();
			}

			bestMoves.push_back( generatedMoves[i] );
		}

		// Do any pruning.

		if( bDoAlphaBetaPruning  &&  kbNewBestLine  &&
				dParentMoveValue - dBestLineValue < dBestUncleLineValue )
		{
			break;
		}
	}

	if( pBestMove != 0 )
	{
		//Assert( bestMoves.size() > 0 );
		srand( time( 0 ) );
		*pBestMove = bestMoves[rand() % bestMoves.size()];
	}

	return( dBestLineValue );
}


CGame::CGame( void )
	: m_KingArchetype( ePieceType_King ),
		m_QueenArchetype( ePieceType_Queen ),
		m_RookArchetype( ePieceType_Rook ),
		m_BishopArchetype( ePieceType_Bishop ),
		m_KnightArchetype( ePieceType_Knight ),
		m_PawnArchetype( ePieceType_Pawn ),
		m_WhitePlayer( 0, *this, m_BlackPlayer ),
		m_BlackPlayer( 1, *this, m_WhitePlayer ),
		m_nPawnCapturableViaEnPassant( -1 )
{
	InitializeBoard();
}


void CGame::InitializeBoard( void )
{
	int i = 0;
	int nVectorSize = 0;

	for( i = 0; i < cnBoardArea; ++i )
	{
		m_anBoard[i] = 0;
	}

	nVectorSize = m_WhitePlayer.mPieces.size();

	for( i = 0; i < nVectorSize; ++i )
	{
		CPiece * pPiece = &m_WhitePlayer.mPieces[i];
		const int knBoardIndex = pPiece->m_nRow * 8 + pPiece->m_nCol;

		m_anBoard[knBoardIndex] = pPiece;
	}

	nVectorSize = m_BlackPlayer.mPieces.size();

	for( i = 0; i < nVectorSize; ++i )
	{
		CPiece * pPiece = &m_BlackPlayer.mPieces[i];
		const int knBoardIndex = pPiece->m_nRow * 8 + pPiece->m_nCol;

		m_anBoard[knBoardIndex] = pPiece;
	}
}


void CGame::PrintBoard( void ) const
{
	// Display row 7 first, row 0 last.

	for( int nRow = 7; nRow >= 0; --nRow )
	{

		for( int nCol = 0; nCol < 8; ++nCol )
		{
			char cOutput = '?';
			const CPiece * const pPiece = m_anBoard[nRow * 8 + nCol];

			if( pPiece == 0 )
			{
				cOutput = ( nRow + nCol ) % 2 == 0 ? '*' : ' ';
			}
			else
			{
				cOutput = pPiece->m_pArchetype->m_Printable;

				if( pPiece->m_Owner.m_knSelfID == 1 )	// It's a black piece.
				{
					cOutput += 'a' - 'A';
				}
			}

			cout << cOutput;
		}

		cout << '\n';
	}
}


void CGame::Play( void ) throw( CException )
{
}


int main( void )
{
	cout << "pdchess2 : Starting..." << endl;

	try
	{
		CAutoPtr<CGame> pGame = new CGame;

		if( pGame == 0 )
		{
			ThrowException( eStatus_ResourceAcquisitionFailed );
		}

		pGame->Play();
	}
	catch( const CException & e )
	{
		cout << "Exception thrown on line " << e.GetLineNumber() << endl;
	}
	catch( ... )
	{
		// Do nothing.
	}

	cout << "pdchess2 : Finished." << endl;

	char cDummy;

	cout << "Enter a character: ";
	cin >> cDummy;

	return( 0 );
} // main()


// **** End of File ****
