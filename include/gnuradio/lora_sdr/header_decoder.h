/* -*- c++ -*- */  // File uses C++ coding conventions adopted by GNU Radio modules.
/*  // Preserve the original license boilerplate supplied with the project.
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


#ifndef INCLUDED_LORA_SDR_HEADER_DECODER_H  // Guard against multiple inclusion of the header decoder interface declaration.
#define INCLUDED_LORA_SDR_HEADER_DECODER_H  // Define the include guard macro so subsequent includes can skip the body.

#include <gnuradio/lora_sdr/api.h>  // Bring in symbol visibility macros specific to the LoRa SDR module.
#include <gnuradio/block.h>  // Include the base class for GNU Radio blocks providing scheduler integration.

namespace gr {  // Begin the GNU Radio namespace scope.
  namespace lora_sdr {  // Enter the LoRa SDR namespace containing custom block definitions.

    /*!  // Begin the Doxygen comment describing the class.
     * \brief <+description of block+>  // Placeholder text from gr_modtool awaiting a detailed description.
     * \ingroup lora_sdr  // Tag the block as part of the LoRa SDR documentation group.
     *
     */
    class LORA_SDR_API header_decoder : virtual public gr::block  // Declare the abstract interface for the header decoder block.
    {
     public:
      typedef std::shared_ptr<header_decoder> sptr;  // Define a convenient shared pointer alias for header decoder instances.

      /*!  // Document the factory helper used to construct instances.
       * \brief Return a shared_ptr to a new instance of lora_sdr::header_decoder.  // Explain that make() produces new decoder objects.
       *
       * To avoid accidental use of raw pointers, lora_sdr::header_decoder's  // Note that the constructor is hidden behind an implementation.
       * constructor is in a private implementation  // Continue the description referencing the private implementation idiom.
       * class. lora_sdr::header_decoder::make is the public interface for  // Clarify that make() is the method to obtain instances.
       * creating new instances.  // Finish the comment describing the factory.
       */
      static sptr make(bool impl_head, uint8_t cr, uint32_t pay_len, bool has_crc, uint8_t ldro, bool print_header);  // Declare the static factory accepting configuration flags for the decoder implementation.
    };

  } // namespace lora_sdr  // Close the inner namespace scope.
} // namespace gr  // Close the outer GNU Radio namespace.

#endif /* INCLUDED_LORA_SDR_HEADER_DECODER_H */  // End the include guard for the header decoder interface.

