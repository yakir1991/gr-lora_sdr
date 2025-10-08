/* -*- c++ -*- */
/*
 * Copyright 2022 Tapparel Joachim @EPFL,TCL.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from "payload_id_inc_impl.h".
#include "payload_id_inc_impl.h"

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Specify the shared pointer typedef associated with payload_id_inc.
    payload_id_inc::sptr
    // Define the c::make function.
    payload_id_inc::make(std::string separator)
    // Open a new scope block.
    {
      // Return gnuradio::get_initial_sptr to the caller.
      return gnuradio::get_initial_sptr
        // Construct a new object with expression (new payload_id_inc_impl(separator));.
        (new payload_id_inc_impl(separator));
    // Close the current scope block.
    }


    /*
     * The private constructor
     */
    // Define the l::payload_id_inc_impl function.
    payload_id_inc_impl::payload_id_inc_impl(std::string separator)
      // Initialize base classes or members with gr::sync_block("payload_id_inc",.
      : gr::sync_block("payload_id_inc",
              // Specify parameter or initializer gr::io_signature::make(0, 0, 0).
              gr::io_signature::make(0, 0, 0),
              // Define the e::make function.
              gr::io_signature::make(0, 0, 0))
    // Open a new scope block.
    {
      // Assign m_separator to separator.
      m_separator = separator;
      // Call message_port_register_in with arguments (pmt::mp("msg_in")).
      message_port_register_in(pmt::mp("msg_in"));
      // Call message_port_register_out with arguments (pmt::mp("msg_out")).
      message_port_register_out(pmt::mp("msg_out"));
      // set_msg_handler(pmt::mp("noise_est"),boost::bind(&mu_detection_impl::noise_handler, this, _1));
      // Define the r function.
      set_msg_handler(pmt::mp("msg_in"), [this](pmt::pmt_t msg)
                      // Execute statement { this->msg_handler(msg); }).
                      { this->msg_handler(msg); });
    // Close the current scope block.
    }

    /*
     * Our virtual destructor.
     */
    // Define the l::~payload_id_inc_impl function.
    payload_id_inc_impl::~payload_id_inc_impl()
    // Open a new scope block.
    {
    // Close the current scope block.
    }
    // Define the l::msg_handler function.
    void payload_id_inc_impl::msg_handler(pmt::pmt_t msg)
      // Open a new scope block.
      {
        // std::cout << "[mu_detection_impl.cc] Noise estimation received: "<<pmt::to_double(noise_est) << '\n';
        // Assign std::string in_msg to pmt::symbol_to_string(msg).
        std::string in_msg = pmt::symbol_to_string(msg);
        // std::string out_msg = removeNumbers(in_msg);
        // Assign std::string out_msg to in_msg.substr(0, in_msg.find(":")+2).
        std::string out_msg = in_msg.substr(0, in_msg.find(":")+2);
        // Assign out_msg to out_msg.append(std::to_string(++m_cnt)).
        out_msg = out_msg.append(std::to_string(++m_cnt)); 
        // Call message_port_pub with arguments (pmt::intern("msg_out"), pmt::string_to_symbol(out_msg)).
        message_port_pub(pmt::intern("msg_out"), pmt::string_to_symbol(out_msg));
      // Close the current scope block.
      }
    // Execute int.
    int
    // Specify parameter or initializer payload_id_inc_impl::work(int noutput_items.
    payload_id_inc_impl::work(int noutput_items,
        // Specify parameter or initializer gr_vector_const_void_star &input_items.
        gr_vector_const_void_star &input_items,
        // Execute gr_vector_void_star &output_items).
        gr_vector_void_star &output_items)
    // Open a new scope block.
    {
  
      // Tell runtime system how many output items we produced.
      // Return 0 to the caller.
      return 0;
    // Close the current scope block.
    }

  // Close the current scope and emit the trailing comment.
  } /* namespace lora_sdr */
// Close the current scope and emit the trailing comment.
} /* namespace gr */

