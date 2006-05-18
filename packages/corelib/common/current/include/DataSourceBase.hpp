/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:39 CET 2006  DataSourceBase.hpp 

                        DataSourceBase.hpp -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be
 
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
 
 

#ifndef CORELIB_DATASOURCE_BASE_HPP
#define CORELIB_DATASOURCE_BASE_HPP

#include <boost/intrusive_ptr.hpp>
#include <map>
#include <string>
#include <os/oro_atomic.h>
#include "pkgconf/os.h"
#ifdef OROINT_OS_CORBA
#include <corba/ExecutionC.h>
#endif
#include "CommandInterface.hpp"

namespace CORBA
{
    class Any;
}

namespace ORO_CoreLib
{
    class TypeInfo;
    class PropertyBag;

  /**
   * @brief The base class for all DataSource's
   *
   * The DataSource is an object containing Data of any type. It's
   * interface is designed for dynamic build-up and destruction of
   * these objects and allowing Commands, Properties etc to use them
   * as 'storage' devices which have the dual \a copy() /\a clone() semantics
   * (which is heavily used by the Orocos Task Infrastructure).
   *
   * @important DataSource's are reference counted and must be allocated on the headp. Use
   * DataSourceBase::shared_ptr or DataSource<T>::shared_ptr to deal
   * with cleanup of allocated DataSources. You are not allowed to delete
   * a DataSource. If you must have the pointer of a DataSource, use
   * the \a .get() method of the \a shared_ptr class. 
   *
   * Once a newly created DataSource is assigned to a \a shared_ptr,
   * it will be deleted when that pointer goes out of scope and is not
   * shared by other \a shared_ptr objects.
   *
   * @see DataSource
   */
  class DataSourceBase
  {
  protected:
      /**
         We keep the refcount ourselves.  We aren't using
         boost::shared_ptr, because boost::intrusive_ptr is better,
         exactly because it can be used with refcounts that are stored
         in the class itself.  Advantages are that the shared_ptr's for
         derived classes use the same refcount, which is of course very
         much desired, and that refcounting happens in an efficient way,
         which is also nice :)
      */
      mutable atomic_t refcount;

      /** the destructor is private.  You are not allowed to delete this
       * class yourself, use a shared pointer !
       */
      virtual ~DataSourceBase();

  public:
      /**
       * Use this type to store a pointer to a DataSourceBase.
       */
      typedef boost::intrusive_ptr<DataSourceBase> shared_ptr;

      /**
       * Use this type to store a const pointer to a DataSourceBase.
       */
      typedef boost::intrusive_ptr<const DataSourceBase> const_ptr;

      DataSourceBase();
      /**
       * Increase the reference count by one.
       */
      void ref() const;
      /**
       * Decrease the reference count by one and delete this on zero.
       */
      void deref() const;

      /**
       * Reset the data to initial values.
       */
      virtual void reset();

      /**
       * Force an evaluation of the DataSourceBase.
       * @return true on successful evaluation.
       * If the DataSource itself contains a boolean, return that boolean.
       */
      virtual bool evaluate() const = 0;

      /**
       * Update the value of this DataSource with the value of an \a other DataSource.
       * Update does a full update of the value, adding extra
       * information if necessary.
       * @return false if the DataSources are of different type OR if the
       * contents of this DataSource can not be updated.
       */
      virtual bool update( const DataSourceBase* other );

      /**
       * Generate a CommandInterface object which will update this DataSource
       * with the value of another DataSource when execute()'ed.
       * @return zero if the DataSource types do not match OR if the
       * contents of this DataSource can not be updated.
       */
      virtual CommandInterface* updateCommand( const DataSourceBase* other);

      /**
       * Update \a part of the value of this DataSource with the value of an \a other DataSource.
       * Update does a partial update of the value, according to \a part, which is 
       * most likely an index or hash value of some type.
       * @return false if the DataSources are of different type OR if the
       * contents of this DataSource can not be partially updated.
       */
      virtual bool updatePart( DataSourceBase* part, DataSourceBase* other );

      /**
       * Generate a CommandInterface object which will partially update this DataSource
       * with the value of another DataSource when execute()'ed. \a part is an index or 
       * hash value of some type.
       * @return zero if the DataSource types do not match OR if the
       * contents of this DataSource can not be partially updated.
       */
      virtual CommandInterface* updatePartCommand( DataSourceBase* part, DataSourceBase* other);

      /**
       * Return a shallow clone of this DataSource. This method
       * returns a duplicate of this instance which re-uses the
       * DataSources this DataSource holds reference to. The
       * clone() function is thus a non-deep copy.
       */
      virtual DataSourceBase* clone() const = 0;

      /**
       * Create a deep copy of this DataSource, unless it is already
       * cloned. Places the association (parent, clone) in \a
       * alreadyCloned.  If the DataSource is non-copyable (for
       * example it represents the Property of a Task ), \a this may
       * be returned.
       */
      virtual DataSourceBase* copy( std::map<const DataSourceBase*, DataSourceBase*>& alreadyCloned ) const = 0;

      /**
       * Return usefull type info in a human readable format.
       */
      virtual std::string getType() const = 0;

      /**
       * Return the Orocos type info object.
       */
      virtual const TypeInfo* getTypeInfo() const = 0;

      /**
       * Return the Orocos type name, without const, pointer or reference
       * qualifiers.
       */
      virtual std::string getTypeName() const = 0;

      /**
       * Creates a CORBA Any object with the \b current value of this
       * DataSource. This does \b not trigger the evaluation() of this
       * data source.
       * @return a valid Any object or nill if this type is
       * not supported.
       */
      virtual CORBA::Any* createAny() const = 0;

      /**
       * Creates a CORBA Any object with the \b current value of this
       * DataSource. This \b does trigger the evaluation() of this
       * data source.
       * @return a valid Any object or nill if this type is
       * not supported.
       */
      virtual CORBA::Any* getAny() const = 0;

      /**
       * Updates the value of this DataSource with the
       * value of a CORBA Any object.
       * @param any The value to update to.
       * @return true if \a any had the correct type.
       */
      virtual bool update(const CORBA::Any& any);

      /**
       * Stream the contents of this object.
       * @see TypeInfo
       */
      std::ostream& write(std::ostream& os);

      /**
       * Get the contents of this object as a string.
       * @see TypeInfo
       */
      std::string toString();

      /**
       * Decompose the contents of this object into properties.
       * @see TypeInfo
       */
      bool decomposeType( PropertyBag& targetbag );
            
      /**
       * Compose the contents of this object from another datasource.
       * @see TypeInfo
       */
      bool composeType( DataSourceBase::shared_ptr source);

#ifdef OROINT_OS_CORBA
      /**
       * Inspect if this DataSource has an Expression server
       * reference.
       */
      virtual bool hasServer() const;

      /**
       * Create a CORBA object which 'mirrors' this DataSource.
       * @return The Expression server if hasServer(), or a 
       * \a new server object reference otherwise.
       * @see Execution.idl
       */
      virtual Orocos::Expression_ptr server() = 0;

      /**
       * Create a CORBA object which 'mirrors' this DataSource.
       * @return The Expression server if hasServer(), or a 
       * \a new server object reference otherwise.
       * @see Execution.idl
       */
      virtual Orocos::Expression_ptr server() const = 0;

      /**
       * Create a CORBA object which 'mirrors' this DataSource.
       * @return The Method server if hasServer(), or a 
       * \a new method object reference otherwise.
       * @see Execution.idl
       */
      virtual Orocos::Method_ptr method() = 0;
#endif

  };

    /**
     * Stream the contents of this object.
     * @see TypeInfo
     */
    std::ostream& operator<<(std::ostream& os, DataSourceBase::shared_ptr dsb );

}

void intrusive_ptr_add_ref(const ORO_CoreLib::DataSourceBase* p );
void intrusive_ptr_release(const ORO_CoreLib::DataSourceBase* p );

#endif
