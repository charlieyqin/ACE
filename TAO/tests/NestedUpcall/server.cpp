// $Id$

// ============================================================================
//
// = LIBRARY
//    TAO/tests
//
// = FILENAME
//    server.cpp
//
// = AUTHOR
//    Chris Cleeland
//
// ============================================================================

#include "tao/corba.h"
#include "reactor_i.h"

int
main (int argc, char* argv[])
{
  ACE_DEBUG ((LM_DEBUG,
              "NestedUpcall server starting\n"));

  TAO_TRY
    {
      // @@ Chris, can you please try to replace most of this boiler
      // plate code with the TAO_Object_Manager stuff that Sumedh has
      // devised?  If we need to generalize his implementation to
      // handle some of the things you do (e.g., create a special
      // POAManager) please let me know.
      CORBA::ORB_var orb =
        CORBA::ORB_init (argc, argv, "server", TAO_TRY_ENV);
      TAO_CHECK_ENV;

      // Initialize the object adapter.
      CORBA::Object_var poa_object =
	orb->resolve_initial_references("RootPOA");

      if (CORBA::is_nil (poa_object.in ()))
	ACE_ERROR_RETURN ((LM_ERROR,
			   " (%P|%t) Unable to initialize the POA.\n"),
			  1);

      PortableServer::POA_var root_poa =
	PortableServer::POA::_narrow (poa_object.in(),
                                      TAO_TRY_ENV);
      TAO_CHECK_ENV;

      PortableServer::POAManager_var poa_manager =
	root_poa->the_POAManager (TAO_TRY_ENV);
      TAO_CHECK_ENV;

      PortableServer::PolicyList policies (2);
      policies.length (2);  
      policies[0] =
	root_poa->create_id_assignment_policy (PortableServer::USER_ID,
                                               TAO_TRY_ENV);
      policies[1] =
	root_poa->create_lifespan_policy (PortableServer::PERSISTENT,
                                          TAO_TRY_ENV);

      // We use a different POA, otherwise the user would have to
      // change the object key each time it invokes the server.
      PortableServer::POA_var poa =
	root_poa->create_POA ("Persistent_POA",
                              poa_manager.in (),
                              policies,
                              TAO_TRY_ENV);  
      TAO_CHECK_ENV;

      // Now that we've got all that done, we need to create our impl
      // and register!
      PortableServer::ObjectId_var id =
        PortableServer::string_to_ObjectId ("Reactor");
      Reactor_i *reactor_impl = 0;
      ACE_NEW_RETURN (reactor_impl, Reactor_i, -1);

      poa->activate_object_with_id (id.in (),
                                    reactor_impl, TAO_TRY_ENV);
      TAO_CHECK_ENV;

      // Stringify the object
      Reactor_var reactor = reactor_impl->_this (TAO_TRY_ENV);
      TAO_CHECK_ENV;

      CORBA::String_var str =
        orb->object_to_string (reactor.in (), TAO_TRY_ENV);
      TAO_CHECK_ENV;

      ACE_DEBUG ((LM_DEBUG,
                  "(%P|%t) The reactor IOR is <%s>\n",
                  str.in ()));
      
      // Run the server now.
      orb->run ();
    }
  TAO_CATCHANY
    {
      TAO_TRY_ENV.print_exception ("orb_init");
      return -1;
    }
  TAO_ENDTRY;

  ACE_DEBUG ((LM_DEBUG, "NestedUpcall server ending\n"));
  return 0;
}
