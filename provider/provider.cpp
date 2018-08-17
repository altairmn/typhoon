#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <json.hpp>
#include <cstring>
#include "zhelpers.hpp"

#include <iostream>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using json = nlohmann::json;

class Pusher {
public:
  zmq::context_t context;
  zmq::socket_t sender;

  Pusher() : context(1), sender(context, ZMQ_PUSH) {
#if defined PORT
    std::string bind_addr = "tcp://localhost:"
      + std::to_string(PORT);
#endif
    sender.connect(bind_addr.c_str());
    std::cout << bind_addr << std::endl;
  }

  void send(std::string _msg) {
    int len = _msg.length();
    zmq::message_t msg (len);
    memcpy(msg.data(), _msg.c_str(), len);
    sender.send(msg);
  }

} pusher;


void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
  // send the message using zmq here
    json j3 = json::parse(msg->get_payload());
    pusher.send(j3.dump());
}

/// Verify that one of the subject alternative names matches the given hostname
bool verify_subject_alternative_name(const char * hostname, X509 * cert) {
    STACK_OF(GENERAL_NAME) * san_names = NULL;
    
    san_names = (STACK_OF(GENERAL_NAME) *) X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (san_names == NULL) {
        return false;
    }
    
    int san_names_count = sk_GENERAL_NAME_num(san_names);
    
    bool result = false;
    
    for (int i = 0; i < san_names_count; i++) {
        const GENERAL_NAME * current_name = sk_GENERAL_NAME_value(san_names, i);
        
        if (current_name->type != GEN_DNS) {
            continue;
        }
        
        char * dns_name = (char *) ASN1_STRING_data(current_name->d.dNSName);
        
        // Make sure there isn't an embedded NUL character in the DNS name
        if (ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name)) {
            break;
        }
        // Compare expected hostname with the CN
        result = (strcasecmp(hostname, dns_name) == 0);
    }
    sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
    
    return result;
}

/// Verify that the certificate common name matches the given hostname
bool verify_common_name(const char * hostname, X509 * cert) {
    // Find the position of the CN field in the Subject field of the certificate
    int common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(cert), NID_commonName, -1);
    if (common_name_loc < 0) {
        return false;
    }
    
    // Extract the CN field
    X509_NAME_ENTRY * common_name_entry = X509_NAME_get_entry(X509_get_subject_name(cert), common_name_loc);
    if (common_name_entry == NULL) {
        return false;
    }
    
    // Convert the CN field to a C string
    ASN1_STRING * common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
    if (common_name_asn1 == NULL) {
        return false;
    }
    
    char * common_name_str = (char *) ASN1_STRING_data(common_name_asn1);
    
    // Make sure there isn't an embedded NUL character in the CN
    if (ASN1_STRING_length(common_name_asn1) != strlen(common_name_str)) {
        return false;
    }
    
    // Compare expected hostname with the CN
    return (strcasecmp(hostname, common_name_str) == 0);
}

/**
 * This code is derived from examples and documentation found ato00po
 * http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/example/cpp03/ssl/client.cpp
 * and
 * https://github.com/iSECPartners/ssl-conservatory
 */
bool verify_certificate(const char * hostname, bool preverified, boost::asio::ssl::verify_context& ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // Retrieve the depth of the current cert in the chain. 0 indicates the
    // actual server cert, upon which we will perform extra validation
    // (specifically, ensuring that the hostname matches. For other certs we
    // will use the 'preverified' flag from Asio, which incorporates a number of
    // non-implementation specific OpenSSL checking, such as the formatting of
    // certs and the trusted status based on the CA certs we imported earlier.
   // int depth = X509_STORE_CTX_get_error_depth(ctx.native_handle());

   // // if we are on the final cert and everything else checks out, ensure that
   // // the hostname is present on the list of SANs or the common name (CN).
   // if (depth == 0 && preverified) {
   //     X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
   //     
   //     if (verify_subject_alternative_name(hostname, cert)) {
   //         return true;
   //     } else if (verify_common_name(hostname, cert)) {
   //         return true;
   //     } else {
   //         return false;
   //     }
   // }

   // return preverified;
   return true;
}

/// TLS Initialization handler
/**
 * WebSocket++ core and the Asio Transport do not handle TLS context creation
 * and setup. This callback is provided so that the end user can set up their
 * TLS context using whatever settings make sense for their application.
 *
 * As Asio and OpenSSL do not provide great documentation for the very common
 * case of connect and actually perform basic verification of server certs this
 * example includes a basic implementation (using Asio and OpenSSL) of the
 * following reasonable default settings and verification steps:
 *
 * - Disable SSLv2 and SSLv3
 * - Load trusted CA certificates and verify the server cert is trusted.
 * - Verify that the hostname matches either the common name or one of the
 *   subject alternative names on the certificate.
 *
 * This is not meant to be an exhaustive reference implimentation of a perfect
 * TLS client, but rather a reasonable starting point for building a secure
 * TLS encrypted WebSocket client.
 *
 * If any TLS, Asio, or OpenSSL experts feel that these settings are poor
 * defaults or there are critically missing steps please open a GitHub issue
 * or drop a line on the project mailing list.
 *
 * Note the bundled CA cert ca-chain.cert.pem is the CA cert that signed the
 * cert bundled with echo_server_tls. You can use print_client_tls with this
 * CA cert to connect to echo_server_tls as long as you use /etc/hosts or
 * something equivilent to spoof one of the names on that cert 
 * (websocketpp.org, for example).
 */
context_ptr on_tls_init(const char * hostname, websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);


        ctx->set_verify_mode(boost::asio::ssl::verify_peer);
        ctx->set_verify_callback(bind(&verify_certificate, hostname, ::_1, ::_2));

        // Here we load the CA certificates of all CA's that this client trusts.
        ctx->load_verify_file("ca-chain.cert.pem");
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}

int main(int argc, char* argv[]) {
    client c;

    std::string hostname = "localhost";
    std::string port = "9002";
    std::string stream;


    if (argc == 4) {
        hostname = argv[1];
        port = argv[2];
        stream = argv[3];
    } else {
        std::cout << "Usage: print_server_tls <hostname> <port>" << std::endl;
        return 1;
    }
    
    std::string uri = "wss://" + hostname + ":" + port + stream;

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        c.set_error_channels(websocketpp::log::elevel::all);

        // Initialize ASIO
        c.init_asio();

        // Register our message handler
        c.set_message_handler(&on_message);
        c.set_tls_init_handler(bind(&on_tls_init, hostname.c_str(), ::_1));

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return 0;
        }

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        c.connect(con);

        c.get_alog().write(websocketpp::log::alevel::app, "Connecting to " + uri);

        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        c.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}
