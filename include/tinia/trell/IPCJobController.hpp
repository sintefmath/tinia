/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "IPCController.hpp"
#include "tinia/jobcontroller/Job.hpp"


namespace tinia {
namespace model { namespace impl { namespace xml {
class XMLHandler;
}}}
namespace trell {


/** Base class for non-interactive trell jobs (i.e. compute jobs).
  *
  * See documentation of MessageBox for details on overriding the virtual
  * functions.
  *
  * Basic usage:
  *
  * \code
  * #include <boost/thread.hpp>
  * #include <iostream>
  * #include "IPCJobController.hpp"
  *
  * class MyJob : public trell::Job
  * {
  * public:
  *     void
  *     operator()()
  *     {
  *         std::cerr << "worker, sleeping for 2 seconds.\n";
  *         sleep( 2 );
  *         std::cerr << "Finishing.\n";
  *         // Tell message box that we are finished
  *         finish();
  *     }
  * protected:
  *     boost::thread  worker;
  *
  *     bool
  *     init()
  *     {
  *         if( trell::Job::init() ) {
  *             boost::thread t( boost::ref( *this ) );
  *             worker = boost::move( t );
  *             return true;
  *         }
  *         else {
  *             return false;
  *         }
  *     }
  *
  *     void
  *     cleanup()
  *     {
  *         worker.join();
  *         trell::Job::cleanup();
  *     }
  * };
  *
  * int
  * main( int argc, char** argv )
  * {
  *     MyJob j;
  *     j.run( argc, argv );
  *     exit( EXIT_SUCCESS );
  * }
  *\endcode
  */
class IPCJobController : public IPCController, public model::StateListener,
      public model::StateSchemaListener
{
public:

    IPCJobController( bool is_master = false );
    ~IPCJobController();

    virtual void
    setJob( jobcontroller::Job* job );


protected:

    /** \copydoc MessageBox::init */
    virtual
    bool
    init();

    /** \copydoc MessageBox::periodic */
    virtual
    bool
    periodic();

    /** \copydoc MessageBox::cleanup */
    virtual
    void
    cleanup();


    /** Handle a get snapshot event. */
    virtual
    bool
    onGetSnapshot( char*               buffer,
                   TrellPixelFormat    pixel_format,
                   const size_t        width,
                   const size_t        height,
                   const std::string&  session,
                   const std::string&  key );
    virtual
    bool
    onGetRenderlist( size_t&             result_size,
                     char*               buffer,
                     const size_t        buffer_size,
                     const std::string&  session,
                     const std::string&  key,
                     const std::string&  timestamp );


    virtual
    bool
    onGetExposedModelUpdate( size_t&             result_size,
                       char*               buffer,
                       const size_t        buffer_size,
                       const std::string&  session,
                       const unsigned int  revision );

    virtual
    bool
    onUpdateState( const char*         buffer,
                   const size_t        buffer_size,
                   const std::string&  session );

    void stateElementModified(model::StateElement *stateElement);
    void stateSchemaElementAdded(model::StateSchemaElement *stateSchemaElement);
    void stateSchemaElementRemoved(model::StateSchemaElement *stateSchemaElement);
    void stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement);

protected:
    boost::shared_ptr<model::ExposedModel>    m_model;
    jobcontroller::Job*                        m_job;
    model::impl::xml::XMLHandler*                m_xmlHandler;
    volatile bool                            m_updateOngoing;

    /** Handles incoming messages (mainly from master job).
      *
      * \copydetails MessageBox::handle
      *
      * An non-interactive job doesn't do much interaction, so there is usually
      * little need to fiddle with this.
      *
      */
    size_t
    handle( trell_message* msg, size_t buf_size );


    std::vector<std::string> m_viewerKeys;

    size_t m_totalViewerKeySize;
};


}
} // of namespace tinia
