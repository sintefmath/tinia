#pragma once
#include "IPCObserver.hpp"
#include "jobobserver/Job.hpp"
#include "policylibxml/XMLHandler.hpp"

namespace Trell {


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
  * #include "IPCJobObserver.hpp"
  *
  * class MyJob : public Trell::Job
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
  *         if( Trell::Job::init() ) {
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
  *         Trell::Job::cleanup();
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
class IPCJobObserver : public IPCObserver, public policylib::StateListener,
      public policylib::StateSchemaListener
{
public:

    IPCJobObserver( bool is_master = false );
    ~IPCJobObserver();

    virtual void
    setJob( jobobserver::Job* job );


protected:

    /** \copydoc MessageBox::init */
    virtual
    bool
    init( const std::string& xml );

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
    onGetPolicyUpdate( size_t&             result_size,
                       char*               buffer,
                       const size_t        buffer_size,
                       const std::string&  session,
                       const unsigned int  revision );

    virtual
    bool
    onUpdateState( const char*         buffer,
                   const size_t        buffer_size,
                   const std::string&  session );

    void stateElementModified(policylib::StateElement *stateElement);
    void stateSchemaElementAdded(policylib::StateSchemaElement *stateSchemaElement);
    void stateSchemaElementRemoved(policylib::StateSchemaElement *stateSchemaElement);
    void stateSchemaElementModified(policylib::StateSchemaElement *stateSchemaElement);

protected:
    std::shared_ptr<policylib::PolicyLib>    m_policyLib;
    jobobserver::Job*                        m_job;
    policylibxml::XMLHandler*                m_xmlHandler;
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
};


}
