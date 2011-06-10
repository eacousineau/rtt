/***************************************************************************
  tag: The SourceWorks  Tue Sep 7 00:55:18 CEST 2010  RTStringTypeInfo.hpp

                        RTStringTypeInfo.hpp -  description
                           -------------------
    begin                : Tue September 07 2010
    copyright            : (C) 2010 The SourceWorks
    email                : peter@thesourceworks.com

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/


#include "../rt_string.hpp"
#include "../types/SequenceTypeInfo.hpp"
#include "../types/TemplateTypeInfo.hpp"

namespace RTT
{
    namespace types
    {
        /**
         * Standard string specialisation that removes decomposition.
         */
        struct RTStringTypeInfo: public SequenceTypeInfo<rt_string, true, TemplateTypeInfo<rt_string, true> >
        {
            typedef TemplateTypeInfo<rt_string, true> TypeInfoBase;

            RTStringTypeInfo() :
                SequenceTypeInfo<rt_string, true, TypeInfoBase> ("rt_string")
            {
            }

            base::AttributeBase* buildVariable(std::string name, int size) const
            {
                rt_string t_init(size, ' '); // we can't use the default char(), which is null !

                // returned type is identical to parent, but we set spaces.
                base::AttributeBase* ret = SequenceTypeInfo<rt_string, true, TypeInfoBase>::buildVariable(name, size);
                Attribute<rt_string> tt = ret;
                tt.set(t_init);
                return ret;
            }

            virtual bool composeType(base::DataSourceBase::shared_ptr source, base::DataSourceBase::shared_ptr result) const
            {
                return result->update( source.get() );
            }

            /**
             * An rt_string is decomposed into itself.
             */
            virtual base::DataSourceBase::shared_ptr decomposeType(base::DataSourceBase::shared_ptr source) const {
                return source;
            }

        };
    }
}
