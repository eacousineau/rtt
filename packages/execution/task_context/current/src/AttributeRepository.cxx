/***************************************************************************
  tag: Peter Soetens  Tue Dec 21 22:43:08 CET 2004  AttributeRepository.cxx 

                        AttributeRepository.cxx -  description
                           -------------------
    begin                : Tue December 21 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
 
 

#include "execution/AttributeRepository.hpp"
#include "corelib/mystd.hpp"

namespace ORO_Execution
{
    using namespace ORO_CoreLib;

  AttributeRepository::AttributeRepository()
      :bag(0)
  {
  }

  AttributeRepository::~AttributeRepository()
  {
      delete bag;
      // we do not claim automatically ownership
      // call clear() manually to delete all contents.
  }

    AttributeRepository* AttributeRepository::copy( std::map<const DataSourceBase*, DataSourceBase*>& repl, bool inst ) const
    {
        AttributeRepository* ar = new AttributeRepository();
        for ( map_t::const_iterator i = values.begin(); i != values.end(); ++i ) {
            ar->setValue(i->first, i->second->copy( repl, inst ) );
        }
        return ar;
    }


  void AttributeRepository::clear()
  {
    for ( map_t::iterator i = values.begin(); i != values.end(); ++i )
      delete i->second;
    values.clear();
    delete bag;
    bag = 0;
  }

  bool AttributeRepository::setValue( const std::string& name,
                                      AttributeBase* value )
  {
    map_t::iterator i = values.find( name );
    if ( i != values.end() )
        return false;
    values[name] = value;
    return true;
  }

    bool AttributeRepository::addProperty( ORO_CoreLib::PropertyBase* pb ) {
        if ( isDefined( pb->getName() ) || (bag && bag->find( pb->getName() ) != 0) )
            return false;
        if ( bag == 0 )
            bag = new ORO_CoreLib::PropertyBag();
        bag->add( pb );
        return true;
    }

  void AttributeRepository::removeValue( const std::string& name )
  {
    map_t::iterator i = values.find( name );
    if ( i != values.end() ) {
        delete i->second;
        values.erase( name );
    }
  }

  AttributeBase* AttributeRepository::getValue( const std::string& name )
  {
    map_t::iterator i = values.find( name );
    if ( i == values.end() ) return 0;
    else return i->second;
  }

  bool AttributeRepository::isDefined( const std::string& name ) const
  {
    return values.find( name ) != values.end();
  }

  bool AttributeRepository::hasAttribute( const std::string& name ) const
  {
    return values.find( name ) != values.end();
  }

  bool AttributeRepository::hasProperty( const std::string& name ) const
  {
      return (bag && bag->find(name) != 0);
  }

    bool AttributeRepository::removeProperty( PropertyBase* p )
    {
        if ( bag && bag->find( p->getName() ) ) {
            bag->remove(p);
            removeValue( p->getName() );
            return true;
        }
        return false;
    }


    std::vector<std::string> AttributeRepository::names() const
    {
        return ORO_std::keys( values );
    }

    PropertyBag* AttributeRepository::properties() const
    {
        return bag;
    }
}
