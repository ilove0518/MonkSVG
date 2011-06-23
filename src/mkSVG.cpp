/*
 *  mkSVG.cpp
 *  MonkSVG
 *
 *  Created by Micah Pearlman on 8/2/10.
 *  Copyright 2010 Zero Vision. All rights reserved.
 *
 */

#include "mkSVG.h"
#include "tinyxml.h"
#include <map>
#include <iterator>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
using namespace boost;

namespace MonkSVG {
	
	
	bool SVG::initialize( ISVGHandler::SmartPtr handler ) {
		_handler = handler;
		
		return true;
	}
	
	bool SVG::read( const char* data ) {
		TiXmlDocument doc;
		
		
		doc.Parse( data );
		
		TiXmlElement* root = doc.RootElement();//doc.FirstChild( "svg" )->ToElement();
		recursive_parse( &doc, root );
		
		return true;
		
	}
	
	bool SVG::read( string& data ) {
		return read( data.c_str() );
	}
	
	void SVG::recursive_parse( TiXmlDocument* doc, TiXmlElement* element ) {
		if ( element ) {
			TiXmlElement* child = element->FirstChildElement();
			recursive_parse( doc, child );
		}
		
		if ( element ) {
			for ( TiXmlElement* sibbling = element; sibbling != 0; sibbling = sibbling->NextSiblingElement() ) {
				string type = sibbling->Value();
				
				if ( type == "g" ) {
					handle_group( sibbling );
					// go through each child path
					//					recursive_parse( doc, sibbling->FirstChildElement() );
//					for ( TiXmlElement* child = sibbling->FirstChildElement(); child != 0; child = child->NextSiblingElement() ) {
//						handle_path( child );
//					}
				}
				
				if ( type == "path" ) {
					handle_path( sibbling );
				}
				if ( type == "rect" ) {
					handle_rect( sibbling );
				}
			}
		}
	}
	
	void SVG::handle_group( TiXmlElement* pathElement ) {
		_handler->onGroupBegin();
		
		// handle transform
		handle_general_parameter( pathElement );
//		string transform;
//		if ( pathElement->QueryStringAttribute( "transform", &transform) == TIXML_SUCCESS ) {
//			parse_path_transform( transform );
//		}
		
		// go through all the children
		TiXmlElement* children = pathElement->FirstChildElement();
		for ( TiXmlElement* child = children; child != 0; child = child->NextSiblingElement() ) {
			string type = child->Value();
			if ( type == "g" ) {
				handle_group( child );
			} else if( type == "path" ) {
				handle_path( child );
			} else 	if ( type == "rect" ) {
				handle_rect( child );
			}

		}
		
		
		
		
		_handler->onGroupEnd();

	}
	
	void SVG::handle_path( TiXmlElement* pathElement ) {
		_handler->onPathBegin();
		string d;
		if ( pathElement->QueryStringAttribute( "d", &d ) == TIXML_SUCCESS ) {
			parse_path_d( d );
		}
		
		handle_general_parameter( pathElement );
		
		_handler->onPathEnd();		
	}
	
	void SVG::handle_rect( TiXmlElement* pathElement ) {
		_handler->onPathBegin();
		
		
		float pos[2];
		if ( pathElement->QueryFloatAttribute( "x", &pos[0] ) == TIXML_SUCCESS ) {
			//parse_path_d( d );
		}
		if ( pathElement->QueryFloatAttribute( "y", &pos[1] ) == TIXML_SUCCESS ) {
			//parse_path_d( d );
		}
		float sz[2];
		if ( pathElement->QueryFloatAttribute( "width", &sz[0] ) == TIXML_SUCCESS ) {
			//parse_path_d( d );
		}
		if ( pathElement->QueryFloatAttribute( "height", &sz[1] ) == TIXML_SUCCESS ) {
			//parse_path_d( d );
		}
		_handler->onPathRect( pos[0], pos[1], sz[0], sz[1] );
		

		handle_general_parameter( pathElement );
		
		
		_handler->onPathEnd();		
		
	}
	
	void SVG::handle_general_parameter( TiXmlElement* pathElement ) {
		string fill; 
		if ( pathElement->QueryStringAttribute( "fill", &fill ) == TIXML_SUCCESS ) {
			_handler->onPathFillColor( string_hex_color_to_uint( fill ) );
		}
		
		
		string stroke;
		if ( pathElement->QueryStringAttribute( "stroke", &stroke) == TIXML_SUCCESS ) {
			_handler->onPathStrokeColor( string_hex_color_to_uint( stroke ) );
		}
		
		string stroke_width;
		if ( pathElement->QueryStringAttribute( "stroke-width", &stroke_width) == TIXML_SUCCESS ) {
			float width = atof( stroke_width.c_str() );
			_handler->onPathStrokeWidth( width );
		}
		
		string style;
		if ( pathElement->QueryStringAttribute( "style", &style) == TIXML_SUCCESS ) {
			parse_path_style( style );
		}
		
		string transform;
		if ( pathElement->QueryStringAttribute( "transform", &transform) == TIXML_SUCCESS ) {
			parse_path_transform( transform );
		}
		string id_;
		if ( pathElement->QueryStringAttribute( "id", &id_) == TIXML_SUCCESS ) {
			_handler->onId( id_ );
			//cout << id_ << endl;
		}
		

	}

	
	
	float SVG::d_string_to_float( char *c, char** str ) {
		while ( isspace(*c) ) {
			c++;
			(*str)++;
		}
		while ( *c == ',' ) {
			c++;
			(*str)++;
		}
		
		return strtof( c, str );
	}
	
	int SVG::d_string_to_int( char *c, char **str ) {
		while ( isspace(*c) ) {
			c++;
			(*str)++;
		}
		while ( *c == ',' ) {
			c++;
			(*str)++;
		}
		
		return strtol( c, str, 10);
		
	}
	
	uint32_t SVG::string_hex_color_to_uint( string& hexstring ) {
		uint32_t color = strtol( hexstring.c_str() + 1, 0, 16 );
		if ( hexstring.length() == 7 ) {	// fix up to rgba if the color is only rgb
			color = color << 8;
			color |= 0x000000ff;
		}
		
		return color;
	}	
	
	
	void SVG::nextState( char** c, char* state ) {
		if ( **c == '\0') {
			*state = 'e';
			return;
		}
		
		while ( isspace(**c) ) {
			if ( **c == '\0') {
				*state = 'e';
				return;
			}
			(*c)++;
		}
		if ( isalpha( **c ) ) {
			*state = **c;
			(*c)++;
//			if ( **c == '\0') {
//				*state = 'e';
//				return;
//			}
			
			if ( islower(*state) ) {	// if lower case then relative coords (see SVG spec)
				_handler->setRelative( true );
			} else {
				_handler->setRelative( false );
			}
		}
		
		//cout << "state: " << *state << endl;
	}
	
	void SVG::parse_path_transform( string& tr )	{
		size_t p = tr.find( "translate" );
		if ( p != string::npos ) {
			size_t left = tr.find( "(" );
			size_t right = tr.find( ")" );
			string values = tr.substr( left+1, right-1 );
			char* c = const_cast<char*>( values.c_str() );
			float x = d_string_to_float( c, &c );
			float y = d_string_to_float( c, &c );
			_handler->onTransformTranslate( x, y );
		} else if ( tr.find( "rotate" ) != string::npos ) {
			size_t left = tr.find( "(" );
			size_t right = tr.find( ")" );
			string values = tr.substr( left+1, right-1 );
			char* c = const_cast<char*>( values.c_str() );
			float a = d_string_to_float( c, &c );
			_handler->onTransformRotate( a );	// ??? radians or degrees ??
		} else if ( tr.find( "matrix" ) != string::npos ) {
			size_t left = tr.find( "(" );
			size_t right = tr.find( ")" );
			string values = tr.substr( left+1, right-1 );
			char* cc = const_cast<char*>( values.c_str() );
			float a = d_string_to_float( cc, &cc );
			float b = d_string_to_float( cc, &cc );
			float c = d_string_to_float( cc, &cc );
			float d = d_string_to_float( cc, &cc );
			float e = d_string_to_float( cc, &cc );
			float f = d_string_to_float( cc, &cc );
			_handler->onTransformMatrix( a, b, c, d, e, f );
		}
	}
	
	void SVG::parse_path_d( string& d ) {
		char* c = const_cast<char*>( d.c_str() );
		char state = *c;
		nextState( &c, &state );
		while ( /**c &&*/ state != 'e' ) {
			
			switch ( state ) {
				case 'm':
				case 'M':
				{
					//c++;
					float x = d_string_to_float( c, &c );
					float y = d_string_to_float( c, &c );
					_handler->onPathMoveTo( x, y );
					nextState(&c, &state);
				}
				break;
					
				case 'l':
				case 'L':
				{
					//c++;
					float x = d_string_to_float( c, &c );
					float y = d_string_to_float( c, &c );
					_handler->onPathLineTo( x, y );
					nextState(&c, &state);
					
				}
				break;
				
				case 'h':
				case 'H':
				{
					float x = d_string_to_float( c, &c );
					_handler->onHorizontalLine( x );
					nextState(&c, &state);

				}
				break;

				case 'v':
				case 'V':
				{
					float y = d_string_to_float( c, &c );
					_handler->onVerticalLine( y );
					nextState(&c, &state);
					
				}
				break;
					
				case 'c':
				case 'C':
				{
					//c++;
					float x1 = d_string_to_float( c, &c );
					float y1 = d_string_to_float( c, &c );
					float x2 = d_string_to_float( c, &c );
					float y2 = d_string_to_float( c, &c );
					float x3 = d_string_to_float( c, &c );
					float y3 = d_string_to_float( c, &c );
					_handler->onPathCubic(x1, y1, x2, y2, x3, y3);
					nextState(&c, &state);
					
				}
				break;
					
				case 'a':
				case 'A':
				{
					float rx = d_string_to_float( c, &c );
					float ry = d_string_to_float( c, &c );
					float x_axis_rotation = d_string_to_float( c, &c );
					int large_arc_flag = d_string_to_int( c, &c );
					int sweep_flag = d_string_to_int( c, &c );
					float x = d_string_to_float( c, &c );;
					float y = d_string_to_float( c, &c );
					_handler->onPathArc( rx, ry, x_axis_rotation, large_arc_flag, sweep_flag, x, y );
					nextState(&c, &state);
					
				}
				break;
					
				case 'z':
				case 'Z':
				{
					//c++;
					_handler->onPathClose();
					nextState(&c, &state);
					
				}
				break;
					
				
				default:
					// BUGBUG: can get stuck here!
					// TODO: figure out the next state if we don't handle a particular state or just dummy handle a state!
					c++;
					break;
			}
		}
	}
	
	// semicolon-separated property declarations of the form "name : value" within the ‘style’ attribute
	void SVG::parse_path_style( string& ps ) {
		map< string, string > style_key_values;
		char_separator<char> values_seperator(";");
		char_separator<char> key_value_seperator(":");
		tokenizer<char_separator<char> > values_tokens( ps, values_seperator );
		BOOST_FOREACH( string values, values_tokens ) {
			tokenizer<char_separator<char> > key_value_tokens( values, key_value_seperator );
			tokenizer<char_separator<char> >::iterator k = key_value_tokens.begin();
			tokenizer<char_separator<char> >::iterator v = k;
			v++;
			//cout << *k << ":" << *v << endl;
			style_key_values[*k] = *v;
		}
		
		map<string, string>::iterator kv = style_key_values.find( string("fill") );
		if ( kv != style_key_values.end() ) {
			if( kv->second != "none" )
				_handler->onPathFillColor( string_hex_color_to_uint( kv->second ) );
		}
		
		kv = style_key_values.find( "stroke" );
		if ( kv != style_key_values.end() ) {
			if( kv->second != "none" )
				_handler->onPathStrokeColor( string_hex_color_to_uint( kv->second ) );
		}
		
		kv = style_key_values.find( "stroke-width" );
		if ( kv != style_key_values.end() ) {
			float width = atof( kv->second.c_str() );
			_handler->onPathStrokeWidth( width );
		}
		
		kv = style_key_values.find( "fill-rule" );
		if ( kv != style_key_values.end() ) {
			_handler->onPathFillRule( kv->second );
		}
		
	}

};