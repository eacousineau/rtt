/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:40 CET 2006  ExecutionEngine.hpp

                        ExecutionEngine.hpp -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be

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


#ifndef ORO_EXECUTION_ENGINE_HPP
#define ORO_EXECUTION_ENGINE_HPP

#include "os/Mutex.hpp"
#include "os/MutexLock.hpp"
#include "os/Condition.hpp"
#include "base/RunnableInterface.hpp"
#include "base/ActivityInterface.hpp"
#include "base/DisposableInterface.hpp"
#include "base/ExecutableInterface.hpp"
#include "internal/List.hpp"
#include <vector>
#include <boost/function.hpp>

#include "rtt-config.h"
#include "internal/rtt-internal-fwd.hpp"

namespace RTT
{

    /**
     * An execution engine serialises (executes one after the other)
     * the execution of all commands, programs, state machines and
     * incomming events for a task.  Any function executing in the
     * same execution engine is guaranteed to be thread-safe with
     * respect to other functions executing in the same execution
     * engine.
     *
     * The ExecutionEngine bundles a internal::CommandProcessor, scripting::ProgramProcessor,
     * scripting::StateMachineProcessor and MessageProcessor.
     *
     * @par Changing the Execution Policy
     * One can subclass this class in order to change the run-time
     * behaviour. Use base::TaskCore::setExecutionEngine in order to
     * install a new ExecutionEngine in a component. All Members of
     * this class are protected and thus accessible in a subclass.
     * @ingroup Processor
     */
    class RTT_API ExecutionEngine
        : public base::RunnableInterface
    {
    public:
        /**
         * Create an execution engine with a internal::CommandProcessor, scripting::ProgramProcessor
         * and StateMachineProcessor.
         * @param owner The base::TaskCore in which this execution engine executes.
         * It may be null, in that case no base::TaskCore owns this execution engine.
         */
        ExecutionEngine( base::TaskCore* owner = 0);

        ~ExecutionEngine();

        /**
         * The base::TaskCore which created this ExecutionEngine.
         */
        base::TaskCore* getParent();

        /**
         * Add a base::TaskCore to execute.
         */
        virtual void addChild(base::TaskCore* tc);

        /**
         * Remove a base::TaskCore from execution.
         */
        virtual void removeChild(base::TaskCore* tc);

        /**
         * Returns the owner of this execution engine.
         */
        base::TaskCore* getTaskCore() const { return taskc; }

        /**
         * Queue and execute (process) a given message. The message is
         * executed in step() or loop() directly after all other
         * queued ActionInterface objects. The constructor parameter
         * \a queue_size limits how many messages can be queued in
         * between step()s or loop().
         *
         * @return true if the message got accepted, false otherwise.
         * @return false when the MessageProcessor is not running or does not accept messages.
         * @see acceptMessages
         */
        virtual bool process(base::DisposableInterface* c);

        /**
         * Run a given function in step() or loop(). The function may only
         * be destroyed after the
         * ExecutionEngine is stopped or removeFunction() was invoked. The number of functions the Processor can
         * run in parallel can be increased with setMaxFunctions().
         * @return false if the Engine is not running or the 'pending' queue is full.
         * @see removeFunction(), setMaxFunctions()
         */
        virtual bool runFunction(base::ExecutableInterface* f);

        /**
         * Remove a running function added with runFunction.
         * This method is only required if the function is to be destroyed
         * and is still present in the Engine.
         */
        virtual bool removeFunction(base::ExecutableInterface* f);

        /**
         * Self-removal for a running function added with runFunction.
         * You must call this variant in case you want yourself to be
         * removed. Equivalent to returning false in ExecutableInterface::execute().
         */
        virtual bool removeSelfFunction(base::ExecutableInterface* f);

        /**
         * Call this if you wish to block on a message arriving in the Execution Engine.
         * Each time one or more messages are processed, waitForMessages will return
         * when pred() returns true.
         * @param pred As long as !pred() blocks the calling thread. If pred() == true
         * when entering this function, returns immediately.
         *
         * This function is for internal use only and is required for asynchronous method invocations.
         */
        void waitForMessages(const boost::function<bool(void)>& pred);

        /**
         * Call this if you wish to block on a function completing in the Execution Engine.
         * Each time a function completes, waitForFunctions will return
         * when pred() returns true.
         * @param pred As long as !pred() blocks the calling thread. If pred() == true
         * when entering this function, returns immediately.
         *
         * This function is for internal use only and is required for asynchronous function invocations.
         */
        void waitForFunctions(const boost::function<bool(void)>& pred);

        /**
         * Stops executing the updateHook of \a task.
         * This is an explicit synchronisation point, which guarantees
         * that updateHook is no longer executed when this function returns true.
         * @param task The Task calling this function and whose updateHook should no
         * longer be executed.
         * @return true if it's updateHook() is no longer being executed, false otherwise.
         */
        bool stopTask(base::TaskCore* task);


        /**
         * Set the 'owner' task in the exception state.
         */
        void setExceptionTask();
    protected:
        /**
         * Call this if you wish to block on a message arriving in the Execution Engine.
         * Each time one or more messages are processed, waitForMessages will return
         * when pred() returns true.
         * @param pred As long as !pred() blocks the calling thread. If pred() == true
         * when entering this function, the returns immediately.
         *
         * This function is for internal use only and is required for asynchronous method invocations.
         *
         * @note waitForMessages requires another thread to execute processMessages()
         * and may therefor not be called from within the component's Thread. Use
         * waitAndProcessMessages() instead.
         */
        void waitForMessagesInternal( boost::function<bool(void)> const& pred);

        /**
         * Call this if you wish to block on a message arriving in the Execution Engine
         * and execute it.
         * @param pred As long as !pred() waits and processes messages. If pred() == true
         * when entering this function, then no messages will be processed and this function
         * returns immediately.
         *
         * This function is for internal use only and is required for asynchronous method invocations.
         *
         * @note waitAndProcessMessages will call in turn this->processMessages() and may as a consequence
         * recurse if we get an asynchronous call-back.
         */
        void waitAndProcessMessages(boost::function<bool(void)> const& pred);

        /**
         * Call this if you wish to block on a function completing in the Execution Engine
         * and execute it.
         * @param pred As long as !pred() waits and processes functions. If pred() == true
         * when entering this function, then no functions will be processed and this function
         * returns immediately.
         *
         * This function is for internal use only and is required for asynchronous function invocations.
         *
         * @note waitAndProcessFunctions will call in turn this->processFunctions() and may as a consequence
         * recurse if we get an asynchronous call-back.
         */
        void waitAndProcessFunctions(boost::function<bool(void)> const& pred);

        /**
         * The parent or 'owner' of this ExecutionEngine, may be null.
         */
        base::TaskCore*     taskc;

        /**
         * Our Message queue
         */
        internal::MWSRQueue<base::DisposableInterface*>* mqueue;

        std::vector<base::TaskCore*> children;

        /**
         * Stores all functions we're executing.
         */
        internal::MWSRQueue<base::ExecutableInterface*>* f_queue;

        os::Mutex msg_lock;
        os::Condition msg_cond;

        void processMessages();
        void processFunctions();
        void processChildren();

        virtual bool initialize();

        /**
         * Executes (in that order) Messages, Functions and updateHook()
         * functions of this TaskContext and its children.
         */
        virtual void step();

        virtual bool breakLoop();

        virtual void finalize();

        virtual bool hasWork();

    };

}
#endif
