// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from "gray_demap_impl.h".
#include "gray_demap_impl.h"
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>

// Start namespace gr scope.
namespace gr {
  // Start namespace lora_sdr scope.
  namespace lora_sdr {

    // Specify the shared pointer typedef associated with gray_demap.
    gray_demap::sptr
    // Define the p::make function.
    gray_demap::make(uint8_t sf)
    // Open a new scope block.
    {
      // Return gnuradio::get_initial_sptr to the caller.
      return gnuradio::get_initial_sptr
        // Construct a new object with expression (new gray_demap_impl(sf));.
        (new gray_demap_impl(sf));
    // Close the current scope block.
    }

    
    /*
     * The private constructor
     */
    // Define the l::gray_demap_impl function.
    gray_demap_impl::gray_demap_impl(uint8_t sf)
      // Initialize base classes or members with gr::sync_block("gray_demap",.
      : gr::sync_block("gray_demap",
              // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(uint32_t)).
              gr::io_signature::make(1, 1, sizeof(uint32_t)),
              // Define the e::make function.
              gr::io_signature::make(1, 1, sizeof(uint32_t)))
    // Open a new scope block.
    {
        // Assign m_sf to sf.
        m_sf = sf;
        // Call set_tag_propagation_policy with arguments (TPP_ONE_TO_ONE).
        set_tag_propagation_policy(TPP_ONE_TO_ONE);
    // Close the current scope block.
    }

    // Define the l::set_sf function.
    void gray_demap_impl::set_sf(uint8_t sf){
      // Assign m_sf to sf.
      m_sf = sf;
    // Close the current scope block.
    } 

    /*
     * Our virtual destructor.
     */
    // Define the l::~gray_demap_impl function.
    gray_demap_impl::~gray_demap_impl()
    // Execute {}.
    {}

    // Execute int.
    int
    // Specify parameter or initializer gray_demap_impl::work(int noutput_items.
    gray_demap_impl::work(int noutput_items,
        // Specify parameter or initializer gr_vector_const_void_star &input_items.
        gr_vector_const_void_star &input_items,
        // Execute gr_vector_void_star &output_items).
        gr_vector_void_star &output_items)
    // Open a new scope block.
    {
      // Assign const uint32_t *in to (const uint32_t *) input_items[0].
      const uint32_t *in = (const uint32_t *) input_items[0];
      // Assign uint32_t *out to (uint32_t *) output_items[0].
      uint32_t *out = (uint32_t *) output_items[0];

      // Declare std::vector<tag_t> tags.
      std::vector<tag_t> tags;

      // Call get_tags_in_window with arguments (tags, 0, 0, noutput_items, pmt::string_to_symbol("configuration")).
      get_tags_in_window(tags, 0, 0, noutput_items, pmt::string_to_symbol("configuration"));
      // Branch when condition (tags.size() > 0) evaluates to true.
      if (tags.size() > 0) {
          //Update cr and sf
          // Assign pmt::pmt_t err_sf to pmt::string_to_symbol("error").
          pmt::pmt_t err_sf = pmt::string_to_symbol("error");
          // Assign int new_sf to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err_sf)).
          int new_sf = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err_sf));
          // Branch when condition (new_sf != m_sf) evaluates to true.
          if (new_sf != m_sf) {
              // Assign m_sf to new_sf.
              m_sf = new_sf;
              // std::cout<<"New sf gray demap "<< static_cast<int>(m_sf) <<std::endl;
          // Close the current scope block.
          }
      // Close the current scope block.
      }
      // Iterate with loop parameters (int i=0;i<noutput_items;i++).
      for(int i=0;i<noutput_items;i++){
        // Only compile the following section when GRLORA_DEBUG is defined.
        #ifdef GRLORA_DEBUG
        // Declare std::cout<<std::hex<<"0x"<<in[i]<<" -->  ".
        std::cout<<std::hex<<"0x"<<in[i]<<" -->  ";
        // Close the preceding conditional compilation block.
        #endif
        // Assign out[i] to in[i].
        out[i]=in[i];
        // Iterate with loop parameters (int j=1;j<m_sf;j++).
        for(int j=1;j<m_sf;j++){
             // Assign out[i] to out[i]^(in[i]>>j).
             out[i]=out[i]^(in[i]>>j);
        // Close the current scope block.
        }
        //do the shift of 1
         // Assign out[i] to mod(out[i]+1,(1<<m_sf)).
         out[i]=mod(out[i]+1,(1<<m_sf));
         // Only compile the following section when GRLORA_DEBUG is defined.
         #ifdef GRLORA_DEBUG
         // Execute statement std::cout<<"0x"<<out[i]<<std::dec<<std::endl.
         std::cout<<"0x"<<out[i]<<std::dec<<std::endl;
         // Close the preceding conditional compilation block.
         #endif
      // Close the current scope block.
      }

      // Return noutput_items to the caller.
      return noutput_items;
    // Close the current scope block.
    }
  // Close the current scope and emit the trailing comment.
  } /* namespace lora */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
