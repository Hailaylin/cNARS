/*
	Parser module generated by unicc from narsese.par.
	DO NOT EDIT THIS FILE MANUALLY, IT WILL GO AWAY!
*/

#if 0 == 0
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#else

#endif

// Include parser control block definitions
#include "narsese.h"

UNICC_SCHAR* _parser::get_lexem( void )
{
#if UNICC_WCHAR || !UNICC_UTF8
	this->lexem = this->buf;
#else
	size_t		size;

	size = wcstombs( (char*)NULL, this->buf, 0 );

	free( this->lexem );

	if( !( this->lexem = (UNICC_SCHAR*)malloc(
			( size + 1 ) * sizeof( UNICC_SCHAR ) ) ) )
	{
		UNICC_OUTOFMEM( this );
		return NULL;
	}

	wcstombs( this->lexem, this->buf, size + 1 );
#endif

#if UNICC_DEBUG	> 2
	fprintf( stderr, "%s: lexem: this->lexem = >" UNICC_SCHAR_FORMAT "<\n",
						UNICC_PARSER, this->lexem );
#endif
	return this->lexem;
}

_ast* _parser::ast_free( _ast* node )
{
	if( !node )
		return NULL;

	this->ast_free( node->child );
	this->ast_free( node->next );

	if( node->token )
		free( node->token );

	free( node );
	return NULL;
}

_ast* _parser::ast_create( const char* emit, UNICC_SCHAR* token )
{
	_ast*	node;

	if( !( node = (_ast*)malloc( sizeof( _ast ) ) ) )
	{
		UNICC_OUTOFMEM( this );
		return node;
	}

	memset( node, 0, sizeof( _ast ) );

	node->emit = emit;

	if( token )
	{
		#if !UNICC_WCHAR
		if( !( node->token = strdup( token ) ) )
		{
			UNICC_OUTOFMEM( this );
			free( node );
			return (_ast*)NULL;
		}
		#else
		if( !( node->token = wcsdup( token ) ) )
		{
			UNICC_OUTOFMEM( this );
			free( node );
			return (_ast*)NULL;
		}
		#endif
	}

	return node;
}

void _parser::ast_print( FILE* stream, _ast* node )
{
	int 		i;
	static int 	rec;

	if( !node )
		return;

	if( !stream )
		stream = stderr;

	while( node )
	{
		for( i = 0; i < rec; i++ )
			fprintf( stream,  " " );

		fprintf( stream, "%s", node->emit );

		if( node->token && strcmp( node->emit, node->token ) != 0 )
			fprintf( stream, " (%s)", node->token );

		fprintf( stream, "\n" );

		rec++;
		this->ast_print( stream, node->child );
		rec--;

		node = node->next;
	}
}

bool _parser::get_act( void )
{
	for( int i = 1; i < this->actions[ this->tos->state ][0] * 3; i += 3 )
	{
		if( this->actions[ this->tos->state ][i] == this->sym )
		{
			if( ( this->act = this->actions[ this->tos->state ][i+1] )
					== UNICC_ERROR )
				return 0; /* Force parse error! */

			this->idx = this->actions[ this->tos->state ][i+2];
			return true;
		}
	}

	/* Default production */
	if( ( this->idx = this->def_prod[ this->tos->state ] ) > -1 )
	{
		this->act = 1; /* Reduce */
		return true;
	}

	return false;
}

bool _parser::get_go( void )
{
	for( int i = 1; i < this->go[ this->tos->state ][0] * 3; i += 3 )
	{
		if( this->go[ this->tos->state ][i] == this->lhs )
		{
			this->act = this->go[ this->tos->state ][ i + 1 ];
			this->idx = this->go[ this->tos->state ][ i + 2 ];
			return true;
		}
	}

	return false;
}

bool _parser::alloc_stack( void )
{
	if( !this->stacksize )
	{
		if( !( this->tos = this->stack = (_tok*)malloc(
				UNICC_MALLOCSTEP * sizeof( _tok ) ) ) )
		{
			UNICC_OUTOFMEM( this );
			return false;
		}

		this->stacksize = UNICC_MALLOCSTEP;
	}
	else if( ( this->tos - this->stack ) == this->stacksize )
	{
		size_t			size = ( this->tos - this->stack );
		_tok*	ptr;

		if( !( ptr = (_tok*)realloc( this->stack,
				( this->stacksize + UNICC_MALLOCSTEP )
					* sizeof( _tok ) ) ) )
		{
			UNICC_OUTOFMEM( this );

			if( this->stack )
			{
				free( this->stack );
				this->tos = this->stack = NULL;
				this->stacksize = 0;
			}

			return false;
		}

		this->tos = this->stack = ptr;
		this->stacksize += UNICC_MALLOCSTEP;
		this->tos += size;
	}

	return true;
}

UNICC_CHAR _parser::get_input( size_t offset )
{
#if UNICC_DEBUG	> 2
	fprintf( stderr, "%s: get input: this->buf + offset = %p this->bufend = %p\n",
				UNICC_PARSER, this->buf + offset, this->bufend );
#endif

	while( this->buf + offset >= this->bufend )
	{
#if UNICC_DEBUG	> 2
			fprintf( stderr, "%s: get input: requiring more input\n",
					UNICC_PARSER );
#endif
		if( !this->buf )
		{
			this->bufend = this->buf = (UNICC_CHAR*)malloc(
				( UNICC_MALLOCSTEP + 1 ) * sizeof( UNICC_CHAR ) );

			if( !this->buf )
			{
				UNICC_OUTOFMEM( this );
				return 0;
			}

			*this->buf = 0;
		}
		else if( *this->buf && !( ( this->bufend - this->buf ) %
					UNICC_MALLOCSTEP ) )
		{
			size_t size	= this->bufend - this->buf;
			UNICC_CHAR*	buf;

			if( !( buf = (UNICC_CHAR*)realloc( this->buf,
						( size + UNICC_MALLOCSTEP + 1 )
							* sizeof( UNICC_CHAR ) ) ) )
			{
				UNICC_OUTOFMEM( this );

				free( this->buf );
				this->buf = NULL;

				return 0;
			}

			this->buf = buf;
			this->bufend = this->buf + size;
		}

		if( this->is_eof || ( *( this->bufend ) = (UNICC_CHAR)UNICC_GETINPUT )
									== this->eof )
		{
#if UNICC_DEBUG	> 2
			fprintf( stderr, "%s: get input: can't get more input, "
						"end-of-file reached\n", UNICC_PARSER );
#endif
			this->is_eof = true;
			return this->eof;
		}
#if UNICC_DEBUG	> 2
		fprintf( stderr, "%s: get input: read char >%c< %d\n",
					UNICC_PARSER, (char)*( this->bufend ), *( this->bufend ) );
#endif

#if UNICC_DEBUG	> 2
		fprintf( stderr, "%s: get input: reading character >%c< %d\n",
					UNICC_PARSER, (char)*( this->bufend ), *( this->bufend ) );
#endif

		*( ++this->bufend ) = 0;
	}

#if UNICC_DEBUG	> 2
	{
		UNICC_CHAR*		chptr;

		fprintf( stderr, "%s: get input: offset = %d\n",
					UNICC_PARSER, offset );
		fprintf( stderr, "%s: get input: buf = >" UNICC_CHAR_FORMAT "<\n",
					UNICC_PARSER, this->buf );
		fprintf( stderr, "%s: get input: returning %d\n",
					UNICC_PARSER, *( this->buf + offset ) );
	}
#endif

	return this->buf[ offset ];
}

void _parser::clear_input( void )
{
	if( this->buf )
	{
		if( this->len )
		{
			/* Update counters for line and column */
			for( int i = 0; i < this->len; i++ )
			{
				if( (char)this->buf[i] == '\n' )
				{
					this->line++;
					this->column = 1;
				}
				else
					this->column++;
			}

#if UNICC_DEBUG > 2
	fprintf( stderr, "%s: clear input: "
		"Clearing %d characters (%d bytes)\n",
			UNICC_PARSER, this->len, this->len * sizeof( UNICC_CHAR ) );
	fprintf( stderr, "%s: clear input: buf = >" UNICC_CHAR_FORMAT "<\n",
			UNICC_PARSER, this->buf, sizeof( UNICC_CHAR ) );
	fprintf( stderr, "%s: clear input: this->bufend >" UNICC_CHAR_FORMAT "<\n",
			UNICC_PARSER, this->bufend );
#endif

			memmove( this->buf, this->buf + this->len,
						( ( this->bufend - ( this->buf + this->len ) ) + 1 + 1 )
							* sizeof( UNICC_CHAR ) );
			this->bufend = this->buf + ( this->bufend - ( this->buf + this->len ) );

#if UNICC_DEBUG	> 2
	fprintf( stderr, "%s: clear input: now buf = >" UNICC_CHAR_FORMAT "<\n",
				UNICC_PARSER, this->buf, sizeof( UNICC_CHAR ) );
	fprintf( stderr, "%s: clear input: now bufend = >" UNICC_CHAR_FORMAT "<\n",
				UNICC_PARSER, this->bufend, sizeof( UNICC_CHAR ) );
#endif
		}
		else
		{
			this->bufend = this->buf;
			*( this->buf ) = 0;
		}
	}

	this->len = 0;
	this->sym = -1;
#if UNICC_DEBUG	> 2
	fprintf( stderr, "%s: clear input: symbol cleared\n", UNICC_PARSER );
#endif
}

#if 80
void _parser::lex( void )
{
	int 		state	= 0;
	size_t		len		= 0;
	int			chr;
	UNICC_CHAR	next;
#if !0
	int			machine	= this->dfa_select[ this->tos->state ];
#else
	int			machine	= 0;
#endif

	next = this->get_input( len );
#if UNICC_DEBUG	> 1
fprintf( stderr, "%s: lex: next = %d\n", UNICC_PARSER, next );
#endif

	if( next == this->eof )
	{
		this->sym = 0;
		return;
	}

	do
	{
#if UNICC_DEBUG	> 1
fprintf( stderr, "%s: lex: next = %d\n", UNICC_PARSER, next );
#endif

		chr = this->dfa_idx[ machine ][ state ];
#if UNICC_DEBUG	> 1
fprintf( stderr, "%s: lex: chr = %d\n", UNICC_PARSER, chr );
#endif

		state = -1;
#if UNICC_DEBUG	> 1
fprintf( stderr, "%s: lex: FIRST next = %d this->dfa_chars[ chr ] = %d, "
			"this->dfa_chars[ chr+1 ] = %d\n", UNICC_PARSER, next,
				this->dfa_chars[ chr ], this->dfa_chars[ chr + 1 ] );
#endif
		while( this->dfa_chars[ chr ] > -1 )
		{
#if UNICC_DEBUG	> 1
fprintf( stderr, "%s: lex: next = %d this->dfa_chars[ chr ] = %d, "
		"this->dfa_chars[ chr+1 ] = %d\n", UNICC_PARSER, next,
			this->dfa_chars[ chr ], this->dfa_chars[ chr + 1 ] );
#endif
			if( next >= this->dfa_chars[ chr ] &&
				next <= this->dfa_chars[ chr+1 ] )
			{
				state = *( this->dfa_trans + ( chr / 2 ) );
#if UNICC_DEBUG	> 1
fprintf( stderr, "%s: lex: state = %d\n", UNICC_PARSER, state );
#endif
				if( this->dfa_accept[ machine ][ state ] > 0 )
				{
					this->len = len + 1;
					this->sym = this->dfa_accept[ machine ][ state ] - 1;

#if UNICC_DEBUG	> 1
fprintf( stderr, "%s: lex: new accepting symbol this->sym = %d greedy = %d\n",
			UNICC_PARSER, this->sym, this->symbols[ this->sym ].greedy );
#endif
					if( this->sym == 0 )
					{
						state = -1; /* test! */
						break;
					}

					/* Stop if matched symbol should be parsed nongreedy */
					if( !this->symbols[ this->sym ].greedy )
					{
						state = -1;
						break;
					}
				}

				next = this->get_input( ++len );
				break;
			}

			chr += 2;
		}
	}
	while( state > -1 && next != this->eof );

	if( this->sym > -1 )
	{
#if UNICC_SEMANTIC_TERM_SEL
		/*
			Execute scanner actions, if existing, but with
			UNICC_ON_SHIFT = 0, so that no memory allocation
			should be performed. This actions should only be
			handled if there are semantic-code dependent
			terminal symbol selections.

			tos is incremented here, if the semantic code
			stores data for the symbol. It won't get lost
			in case of a shift.
		*/
		this->alloc_stack();
		this->tos++;

		next = this->buf[ this->len ];
		this->buf[ this->len ] = 0;

#define UNICC_ON_SHIFT 	0
		switch( this->sym )
		{


			default:
				break;
		}
#undef UNICC_ON_SHIFT

		this->buf[ this->len ] = next;

		this->tos--;
#endif /* UNICC_SEMANTIC_TERM_SEL */
	}

#if UNICC_DEBUG	> 1
fprintf( stderr, "%s: lex: function exits, this->sym = %d, this->len = %d\n",
			UNICC_PARSER, this->sym, this->len );
#endif
}
#endif

bool _parser::get_sym( void )
{
	this->sym = -1;
	this->len = 0;

#if 0
	do
	{
#endif
#if !0

#if UNICC_DEBUG > 2
		fprintf( stderr, "%s: get sym: state = %d dfa_select = %d\n",
					UNICC_PARSER, this->tos->state,
						_dfa_select[ this->tos->state ] );
#endif

		if( this->dfa_select[ this->tos->state ] > -1 )
			this->lex();
		/*
		 * If there is no DFA state machine,
		 * try to identify the end-of-file symbol.
		 * If this also fails, a parse error will
		 * raise.
		 */
		else if( this->get_input( 0 ) == this->eof )
			this->sym = 0;
#else
		this->lex();
#endif /* !0 */

#if 0

		if( this->sym > -1 && this->symbols[ this->sym ].whitespace )
		{
			UNICC_CLEARIN( this );
			continue;
		}

		break;
	}
	while( 1 );
#endif /* 0 */

	return this->sym > -1;
}

#if UNICC_STACKDEBUG
void _parser::dbg_stack( FILE* out, _tok* stack, _tok* tos )
{
	fprintf( out, "%s: Stack Dump: ", UNICC_PARSER );

	for( ; stack <= tos; stack++ )
	{
		fprintf( out, "%d%s%s%s ", stack->state,
			stack->symbol ? " (" : "",
			stack->symbol ? stack->symbol->name : "",
			stack->symbol ? ")" : "" );
	}

	fprintf( out, "\n" );
}
#endif /* UNICC_STACKDEBUG */

bool _parser::handle_error( FILE* _dbg )
{
	if( !this->error_delay )
	{
#if UNICC_DEBUG
		fprintf( _dbg, "%s: !!!PARSE ERROR!!!\n"
				"%s: error recovery: current token %d (%s)\n",
					UNICC_PARSER, UNICC_PARSER, this->sym,
						( ( this->sym >= 0 ) ?
							this->symbols[ this->sym ].name :
								"(null)" ) );

		fprintf( _dbg,
				"%s: error recovery: expecting ", UNICC_PARSER );

		for( int i = 1; i < this->actions[ this->tos->state ][0] * 3; i += 3 )
		{
			fprintf( _dbg, "%d (%s)%s",
				this->actions[ this->tos->state ][i],
				this->symbols[ this->actions[ this->tos->state ][i] ].name,
				( i == this->actions[ this->tos->state ][0] * 3 - 3 ) ?
						"\n" : ", " );
		}

		fprintf( _dbg, "\n%s: error recovery: error_delay is %d, %s\n",
					UNICC_PARSER, this->error_delay,
					( this->error_delay ? "error recovery runs silently" :
						"error is reported before its recover!" ) );
#endif
	}

#if -1 < 0
	/* No error token defined? Then exit here... */

#if UNICC_DEBUG
	fprintf( _dbg,
		"%s: error recovery: No error resync token used by grammar, "
			"exiting parser.\n", UNICC_PARSER );
#endif

	UNICC_PARSE_ERROR( this );
	this->error_count++;

	return 1;
#else

#if UNICC_DEBUG
	fprintf( _dbg, "%s: error recovery: "
		"trying to recover...\n", UNICC_PARSER );
#if UNICC_STACKDEBUG
	_dbg_stack( _dbg, this->stack, this->tos );
#endif
#endif

	/* Remember previous symbol, or discard it */
	if( this->error_delay != UNICC_ERROR_DELAY )
		this->old_sym = this->sym;
	else
	{
		this->old_sym = -1;
		this->len = 1;
		UNICC_CLEARIN( this );
	}

	/* Try to shift on error resync */
	this->sym = -1;

	while( this->tos >= this->stack )
	{
#if UNICC_DEBUG
		fprintf( _dbg, "%s: error recovery: in state %d, trying "
				"to shift error resync token...\n",
					UNICC_PARSER, this->tos->state );
#if UNICC_STACKDEBUG
		this->dbg_stack( _dbg, this->stack, this->tos );
#endif
#endif
		if( _get_act( this ) )
		{
			/* Shift */
			if( this->act & UNICC_SHIFT )
			{
#if UNICC_DEBUG
				fprintf( _dbg, "%s: error recovery: "
							"error resync shifted\n", UNICC_PARSER );
#endif
				break;
			}
		}

#if UNICC_DEBUG
		fprintf( _dbg, "%s: error recovery: failed, "
					"discarding token '%s'\n", UNICC_PARSER,
						this->tos->symbol ?
							this->tos->symbol->name : "NULL" );
#endif

		/* Discard one token from stack */
		/* TODO: Discarded token memory (semantic action) */
		this->tos--;
	}

	if( this->tos <= this->stack )
	{
#if UNICC_DEBUG
	fprintf( _dbg, "%s: error recovery: "
				"Can't recover this issue, stack is empty.\n",
					UNICC_PARSER );
#endif
		UNICC_PARSE_ERROR( this );
		this->error_count++;

		return true;
	}

#if UNICC_DEBUG
	fprintf( _dbg, "%s: error recovery: "
				"trying to continue with modified parser state\n",
					UNICC_PARSER );
#endif

	this->error_delay = UNICC_ERROR_DELAY + 1;

	return false;

#endif /* -1 >= 0 */
}

int _parser::parse( void )
{
	int			ret;
	_ast*		node;
	_ast*		lnode;

#if UNICC_DEBUG
	_vtype*		vptr;
	FILE* 				_dbg = stderr;
#endif

	// Initialize parser
	this->stacksize = 0;
	if( !this->alloc_stack() )
		return (int)NULL;

	memset( this->tos, 0, sizeof( _tok ) );

	this->act = UNICC_SHIFT;
	this->is_eof = false;
	this->sym = this->old_sym = -1;
	this->line = this->column = 1;

	memset( &this->test, 0, sizeof( _vtype ) );

	// Begin of main parser loop
	while( true )
	{
		// Reduce
		while( this->act & UNICC_REDUCE )
		{
#if UNICC_DEBUG
			fprintf( _dbg, "%s: << "
					"reducing by production %d (%s)\n",
						UNICC_PARSER, this->idx,
							this->productions[ this->idx ].definition );
#endif
			// Set default left-hand side
			this->lhs = this->productions[ this->idx ].lhs;

			// Run reduction code
			memset( &( this->ret ), 0, sizeof( _vtype ) );

			switch( this->idx )
			{

			}

			// Drop right-hand side, collect AST nodes
			node = NULL;

			for( int i = 0; i < this->productions[ this->idx ].length; i++ )
			{
				if( this->tos->node )
				{
					if( node )
					{
						while( node->prev )
							node = node->prev;

						node->prev = this->tos->node;
						this->tos->node->next = node;
					}

					node = this->tos->node;
					this->tos->node = NULL;
				}

				this->tos--;
			}

			// Chain collected AST nodes
			if( node )
			{
				if( lnode = this->tos->node )
				{
					while( lnode->next )
						lnode = lnode->next;

					lnode->next = node;
					node->prev = lnode;
				}
				else
					this->tos->node = node;
			}

			// Generate AST node?
			if( *this->productions[ this->idx ].emit )
			{
				if( ( node = this->ast_create(
								this->productions[ this->idx ].emit, NULL ) ) )
				{
					node->child = this->tos->node;
					this->tos->node = node;
				}
			}

			// Enforced error in semantic actions?
			if( this->act == UNICC_ERROR )
				break;

			// Goal symbol reduced, and stack is empty?
			if( this->lhs == 59 && this->tos == this->stack )
			{
				memcpy( &( this->tos->value ), &( this->ret ),
							sizeof( _vtype ) );
				this->ast = this->tos->node;

				UNICC_CLEARIN( this );

				this->act = UNICC_SUCCESS;

				#if UNICC_DEBUG
				fprintf( stderr, "%s: goal symbol reduced, exiting parser\n",
						UNICC_PARSER );
				#endif
				break;
			}

			#if UNICC_DEBUG
			fprintf( _dbg, "%s: after reduction, "
						"shifting nonterminal %d (%s)\n",
							UNICC_PARSER, this->lhs,
								this->symbols[ this->lhs ].name );
			#endif

			this->get_go();

			this->tos++;
			this->tos->node = NULL;

			memcpy( &( this->tos->value ), &( this->ret ),
						sizeof( _vtype ) );
			this->tos->symbol = &( this->symbols[ this->lhs ] );
			this->tos->state = ( this->act & UNICC_REDUCE ) ? -1 : this->idx;
			this->tos->line = this->line;
			this->tos->column = this->column;
		}

		if( this->act == UNICC_SUCCESS || this->act == UNICC_ERROR )
			break;

		/* If in error recovery, replace old-symbol */
		if( this->error_delay == UNICC_ERROR_DELAY
				&& ( this->sym = this->old_sym ) < 0 )
		{
			/* If symbol is invalid, try to find new token */
			#if UNICC_DEBUG
			fprintf( _dbg, "%s: error recovery: "
				"old token invalid, requesting new token\n",
						UNICC_PARSER );
			#endif

			while( !this->get_sym() )
			{
				/* Skip one character */
				this->len = 1;

				UNICC_CLEARIN( this );
			}

			#if UNICC_DEBUG
			fprintf( _dbg, "%s: error recovery: "
				"new token %d (%s)\n", UNICC_PARSER, this->sym,
					this->symbols[ this->sym ].name );
			#endif
		}
		else
			this->get_sym();

#if UNICC_DEBUG
		fprintf( _dbg, "%s: current token %d (%s)\n",
					UNICC_PARSER, this->sym,
						( this->sym < 0 ) ? "(null)" :
							this->symbols[ this->sym ].name );
#endif

		/* Get action table entry */
		if( !this->get_act() )
		{
			/* Error state, try to recover */
			if( this->handle_error(
#if UNICC_DEBUG
					_dbg
#else
					NULL
#endif
					) )
				break;
		}

#if UNICC_DEBUG
		fprintf( _dbg,
			"%s: sym = %d (%s) [len = %d] tos->state = %d act = %s idx = %d\n",
				UNICC_PARSER, this->sym,
					( ( this->sym >= 0 ) ?
						this->symbols[ this->sym ].name :
							"(invalid symbol id)" ),
					this->len, this->tos->state,
						( ( this->act == UNICC_SHIFT & UNICC_REDUCE ) ?
								"shift/reduce" :
							( this->act & UNICC_SHIFT ) ?
									"shift" : "reduce" ), this->idx );
#if UNICC_STACKDEBUG
		this->dbg_stack( _dbg, this->stack, this->tos );
#endif
#endif

		/* Shift */
		if( this->act & UNICC_SHIFT )
		{
			this->next = this->buf[ this->len ];
			this->buf[ this->len ] = '\0';

#if UNICC_DEBUG
			fprintf( _dbg, "%s: >> shifting terminal %d (%s)\n",
			UNICC_PARSER, this->sym, this->symbols[ this->sym ].name );
#endif

			if( !this->alloc_stack() )
				return (int)NULL;

			this->tos++;
			this->tos->node = NULL;

			/*
				Execute scanner actions, if existing.
				Here, UNICC_ON_SHIFT is set to 1, so that shifting-
				related operations will be performed.
			*/
#define UNICC_ON_SHIFT	1
			switch( this->sym )
			{


				default:
					( ( this->tos - 0 )->value ) = this->get_input( 0 );
					break;
			}
#undef UNICC_ON_SHIFT

			this->tos->state = ( this->act & UNICC_REDUCE ) ? -1 : this->idx;
			this->tos->symbol = &( this->symbols[ this->sym ] );
			this->tos->line = this->line;
			this->tos->column = this->column;

			if( *this->tos->symbol->emit )
				this->tos->node = this->ast_create(
									this->tos->symbol->emit,
										this->get_lexem() );
			else
				this->tos->node = NULL;

			this->buf[ this->len ] = this->next;

			/* Perform the shift on input */
			if( this->sym != 0 && this->sym != -1 )
			{
				UNICC_CLEARIN( this );
				this->old_sym = -1;
			}

			if( this->error_delay )
				this->error_delay--;
		}
	}

	#if UNICC_DEBUG
	fprintf( _dbg, "%s: parse completed with %d errors\n",
		UNICC_PARSER, this->error_count );
	#endif

	// Save return value
	ret = ( ( this->tos - 0 )->value );

	// Clean up parser control block
	UNICC_CLEARIN( this );

	free( this->stack );
	this->stack = NULL;

#if UNICC_UTF8
	free( this->lexem );
	this->lexem = NULL;
#endif

	return ret;
}





/* Create Main? */
#if 2 == 0
	#ifndef UNICC_MAIN
	#define UNICC_MAIN 	1
	#endif
#else
	#ifndef UNICC_MAIN
	#define UNICC_MAIN 	0
	#endif
#endif

#if UNICC_MAIN
int main( int argc, char** argv )
{
#define UNICCMAIN_SILENT		1
#define UNICCMAIN_ENDLESS		2
#define UNICCMAIN_LINEMODE		4
#define UNICCMAIN_SYNTAXTREE	8
#define UNICCMAIN_AUGSYNTAXTREE	16

	char*				opt;
	int					flags	= 0;
	_parser*	parser = new _parser;

#ifdef LC_ALL
	setlocale( LC_ALL, "" );
#endif

	/* Get command-line options */
	for( int i = 1; i < argc; i++ )
	{
		if( *(argv[i]) == '-' )
		{
			opt = argv[i] + 1;

			/* Long option coming? */
			if( *opt == '-' )
			{
				opt++;

				if( !strcmp( opt, "silent" ) )
					flags |= UNICCMAIN_SILENT;
				else if( !strcmp( opt, "endless" ) )
					flags |= UNICCMAIN_ENDLESS;
				else if( !strcmp( opt, "line-mode" ) )
					flags |= UNICCMAIN_LINEMODE;
				else
				{
					fprintf( stderr, "Unknown option '--%s'\n", argv[i] );
					return 1;
				}
			}

			for( ; *opt; opt++ )
			{
				if( *opt == 's' )
					flags |= UNICCMAIN_SILENT;
				else if( *opt == 'e' )
					flags |= UNICCMAIN_ENDLESS;
				else if( *opt == 'l' )
					flags |= UNICCMAIN_LINEMODE;
				else
				{
					fprintf( stderr, "Unknown option '-%c'\n", *opt );
					return 1;
				}
			}
		}
	}

	if( flags & UNICCMAIN_LINEMODE )
		parser->eof = '\n';
	else
		parser->eof = EOF;

	/* Parser invocation loop */
	do
	{
		if( !( flags & UNICCMAIN_SILENT ) )
			printf( "\nok\n" );

		parser->parse();

		/* Print AST */
		if( parser->ast )
		{
			parser->ast_print( stderr, parser->ast );
			parser->ast = parser->ast_free( parser->ast );
		}
	}
	while( flags & UNICCMAIN_ENDLESS );

	return 0;
}
#endif

