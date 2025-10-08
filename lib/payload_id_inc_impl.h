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

// Compile the following section when INCLUDED_LORA_SDR_PAYLOAD_ID_INC_IMPL_H is not defined.
#ifndef INCLUDED_LORA_SDR_PAYLOAD_ID_INC_IMPL_H
// Define macro INCLUDED_LORA_SDR_PAYLOAD_ID_INC_IMPL_H.
#define INCLUDED_LORA_SDR_PAYLOAD_ID_INC_IMPL_H

// Bring in declarations from <gnuradio/lora_sdr/payload_id_inc.h>.
#include <gnuradio/lora_sdr/payload_id_inc.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Declare class payload_id_inc_impl.
    class payload_id_inc_impl : public payload_id_inc
    // Open a new scope block.
    {
     // Begin private section of the class declaration.
     private:
      // Declare std::string m_separator.
      std::string m_separator;
      // Execute statement void msg_handler(pmt::pmt_t msg).
      void msg_handler(pmt::pmt_t msg);
      // Assign int m_cnt to 0.
      int m_cnt=0;

     // Begin public section of the class declaration.
     public:
      // Call payload_id_inc_impl with arguments (std::string separator).
      payload_id_inc_impl(std::string separator);
      // Call ~payload_id_inc_impl with arguments ().
      ~payload_id_inc_impl();

      // Where all the action really happens
      // Start declaration for int work with arguments listed on subsequent lines.
      int work(
              // Specify parameter or initializer int noutput_items.
              int noutput_items,
              // Specify parameter or initializer gr_vector_const_void_star &input_items.
              gr_vector_const_void_star &input_items,
              // Execute gr_vector_void_star &output_items.
              gr_vector_void_star &output_items
      // Execute statement ).
      );
    // Close the current scope and emit the trailing comment.
    };

  // Close the current scope block. (inline comment notes: namespace lora_sdr)
  } // namespace lora_sdr
// Close the current scope block. (inline comment notes: namespace gr)
} // namespace gr

// Close the preceding conditional compilation block.
#endif /* INCLUDED_LORA_SDR_PAYLOAD_ID_INC_IMPL_H */

