// Only compile the following section when HAVE_CONFIG_H is defined.
#ifdef HAVE_CONFIG_H
// Bring in declarations from "config.h".
#include "config.h"
// Close the preceding conditional compilation block.
#endif

// Bring in declarations from <gnuradio/io_signature.h>.
#include <gnuradio/io_signature.h>
// Bring in declarations from "interleaver_impl.h".
#include "interleaver_impl.h"
// Bring in declarations from <gnuradio/lora_sdr/utilities.h>.
#include <gnuradio/lora_sdr/utilities.h>

// Start namespace gr scope.
namespace gr
// Open a new scope block.
{
  // Start namespace lora_sdr scope.
  namespace lora_sdr
  // Open a new scope block.
  {

    // Specify the shared pointer typedef associated with interleaver.
    interleaver::sptr
    // Define the r::make function.
    interleaver::make(uint8_t cr, uint8_t sf, uint8_t ldro, int bw)
    // Open a new scope block.
    {
      // Return gnuradio::get_initial_sptr(new interleaver_impl(cr, sf, ldro, bw)) to the caller.
      return gnuradio::get_initial_sptr(new interleaver_impl(cr, sf, ldro, bw));
    // Close the current scope block.
    }

    /*
     * The private constructor
     */
    // Define the l::interleaver_impl function.
    interleaver_impl::interleaver_impl(uint8_t cr, uint8_t sf, uint8_t ldro, int bw)
        // Initialize base classes or members with gr::block("interleaver",.
        : gr::block("interleaver",
                    // Specify parameter or initializer gr::io_signature::make(1, 1, sizeof(uint8_t)).
                    gr::io_signature::make(1, 1, sizeof(uint8_t)),
                    // Define the e::make function.
                    gr::io_signature::make(1, 1, sizeof(uint32_t)))
    // Open a new scope block.
    {
      // Assign m_sf to sf.
      m_sf = sf;
      // Assign m_cr to cr.
      m_cr = cr;
      // Assign m_bw to bw.
      m_bw = bw;
      // Assign m_ldro_mode to ldro.
      m_ldro_mode = ldro;
      // Branch when condition (ldro == AUTO) evaluates to true.
      if (ldro == AUTO){
        // Assign m_ldro to (float)(1u<<sf)*1e3/bw > LDRO_MAX_DURATION_MS.
        m_ldro = (float)(1u<<sf)*1e3/bw > LDRO_MAX_DURATION_MS;
      // Close the current scope block.
      } 
      // Handle the alternative branch when previous conditions fail.
      else
      // Open a new scope block.
      {
        // Assign m_ldro to ldro.
        m_ldro = ldro;
      // Close the current scope block.
      }
      // Assign cw_cnt to 0.
      cw_cnt = 0;

      // Call set_tag_propagation_policy with arguments (TPP_DONT).
      set_tag_propagation_policy(TPP_DONT);
      // Assign m_has_config_tag to false.
      m_has_config_tag = false;
    // Close the current scope block.
    }

    // Define the l::set_cr function.
    void interleaver_impl::set_cr(uint8_t cr){
      // Assign m_cr to cr.
      m_cr = cr;
    // Close the current scope block.
    } 

    // Define the l::set_sf function.
    void interleaver_impl::set_sf(uint8_t sf){
      // Assign m_sf to sf.
      m_sf = sf;
      // Branch when condition (m_ldro_mode == AUTO) evaluates to true.
      if (m_ldro_mode == AUTO){
        //m_ldro = (float)(1u<<sf)*1e3/bw > LDRO_MAX_DURATION_MS;
        // Assign m_ldro to (float)(1u<<m_sf)*1e3/m_bw > LDRO_MAX_DURATION_MS.
        m_ldro = (float)(1u<<m_sf)*1e3/m_bw > LDRO_MAX_DURATION_MS;
      // Close the current scope block.
      }
      // Handle the alternative branch when previous conditions fail.
      else {
        // Assign m_ldro to m_ldro_mode.
        m_ldro = m_ldro_mode;
      // Close the current scope block.
      }
    // Close the current scope block.
    } 

    // Define the l::get_cr function.
    uint8_t interleaver_impl::get_cr(){
      // Return m_cr to the caller.
      return m_cr;
    // Close the current scope block.
    } 

    /*
     * Our virtual destructor.
     */
    // Define the l::~interleaver_impl function.
    interleaver_impl::~interleaver_impl()
    // Open a new scope block.
    {
    // Close the current scope block.
    }

    // Execute void.
    void
    // Define the l::forecast function.
    interleaver_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
    // Open a new scope block.
    {
      // Assign ninput_items_required[0] to 1.
      ninput_items_required[0] = 1;
    // Close the current scope block.
    }

    // Define the l::update_var function.
    void interleaver_impl::update_var(int new_cr, int new_sf, int new_bw)
    // Open a new scope block.
    {
      // Branch when condition (new_cr != m_cr) evaluates to true.
      if (new_cr != m_cr) {
          // Assign m_cr to new_cr.
          m_cr = new_cr;
          // std::cout<<"New cr Interleaver "<< static_cast<int>(m_cr) <<std::endl;
      // Close the current scope block.
      }
      // Branch when condition (new_sf != m_sf) evaluates to true.
      if (new_sf != m_sf) {
          // Assign m_sf to new_sf.
          m_sf = new_sf;
          // std::cout<<"New sf Interleaver "<< static_cast<int>(m_sf) <<std::endl;
      // Close the current scope block.
      }
      // Branch when condition (new_bw != m_bw) evaluates to true.
      if (new_bw != m_bw) {
          // Assign m_bw to new_bw.
          m_bw = new_bw;
          // std::cout<<"New bw Interleaver "<< static_cast<int>(m_bw) <<std::endl;
      // Close the current scope block.
      }
      // Branch when condition (m_ldro_mode == AUTO) evaluates to true.
      if (m_ldro_mode == AUTO){
        //m_ldro = (float)(1u<<sf)*1e3/bw > LDRO_MAX_DURATION_MS;
        // Assign m_ldro to (float)(1u<<m_sf)*1e3/m_bw > LDRO_MAX_DURATION_MS.
        m_ldro = (float)(1u<<m_sf)*1e3/m_bw > LDRO_MAX_DURATION_MS;
      // Close the current scope block.
      }
      // Handle the alternative branch when previous conditions fail.
      else {
        // Assign m_ldro to m_ldro_mode.
        m_ldro = m_ldro_mode;
      // Close the current scope block.
      }
    // Close the current scope block.
    }

    // Execute int.
    int
    // Specify parameter or initializer interleaver_impl::general_work(int noutput_items.
    interleaver_impl::general_work(int noutput_items,
                                   // Specify parameter or initializer gr_vector_int &ninput_items.
                                   gr_vector_int &ninput_items,
                                   // Specify parameter or initializer gr_vector_const_void_star &input_items.
                                   gr_vector_const_void_star &input_items,
                                   // Execute gr_vector_void_star &output_items).
                                   gr_vector_void_star &output_items)
    // Open a new scope block.
    {
      // Assign const uint8_t *in to (const uint8_t *)input_items[0].
      const uint8_t *in = (const uint8_t *)input_items[0];
      // Assign uint32_t *out to (uint32_t *)output_items[0].
      uint32_t *out = (uint32_t *)output_items[0];
      // Assign int nitems_to_process to ninput_items[0].
      int nitems_to_process = ninput_items[0];

      // read tags
      // Declare std::vector<tag_t> tags.
      std::vector<tag_t> tags;
      // Call get_tags_in_window with arguments (tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_len")).
      get_tags_in_window(tags, 0, 0, ninput_items[0], pmt::string_to_symbol("frame_len"));
      // Branch when condition (tags.size()) evaluates to true.
      if (tags.size())
      // Open a new scope block.
      {
        // Branch when condition (tags[0].offset != nitems_read(0)) evaluates to true.
        if (tags[0].offset != nitems_read(0))
          // Assign nitems_to_process to tags[0].offset - nitems_read(0).
          nitems_to_process = tags[0].offset - nitems_read(0);
        // Handle the alternative branch when previous conditions fail.
        else
        // Open a new scope block.
        {
          // Branch when condition (tags.size() >= 2) evaluates to true.
          if (tags.size() >= 2){
            // Assign nitems_to_process to tags[1].offset - tags[0].offset.
            nitems_to_process = tags[1].offset - tags[0].offset;
          // Close the current scope block.
          }
          // Assign cw_cnt to 0.
          cw_cnt = 0;
          // Assign m_frame_len to pmt::to_long(tags[0].value).
          m_frame_len = pmt::to_long(tags[0].value);
          // Assign m_framelen_tag to tags[0].
          m_framelen_tag =  tags[0];
          // Call get_tags_in_window with arguments (tags, 0, 0, 1, pmt::string_to_symbol("configuration")).
          get_tags_in_window(tags, 0, 0, 1, pmt::string_to_symbol("configuration"));
          // Branch when condition (tags.size()>0) evaluates to true.
          if(tags.size()>0)
          // Open a new scope block.
          {
            // Assign m_has_config_tag to true.
            m_has_config_tag = true;
            // Assign m_config_tag to tags[0].
            m_config_tag = tags[0];
            // Assign m_config_tag.offset to nitems_written(0).
            m_config_tag.offset = nitems_written(0); 

            // Assign pmt::pmt_t err_cr to pmt::string_to_symbol("error").
            pmt::pmt_t err_cr = pmt::string_to_symbol("error");
            // Assign pmt::pmt_t err_sf to pmt::string_to_symbol("error").
            pmt::pmt_t err_sf = pmt::string_to_symbol("error");
            // Assign pmt::pmt_t err_bw to pmt::string_to_symbol("error").
            pmt::pmt_t err_bw = pmt::string_to_symbol("error");
            // Assign int new_cr to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err_cr)).
            int new_cr = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("cr"), err_cr));
            // Assign int new_sf to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err_sf)).
            int new_sf = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("sf"), err_sf));
            // Assign int new_bw to pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("bw"), err_bw)).
            int new_bw = pmt::to_long(pmt::dict_ref(tags[0].value, pmt::string_to_symbol("bw"), err_bw));
            // Call update_var with arguments (new_cr, new_sf, new_bw).
            update_var(new_cr, new_sf, new_bw);
          // Close the current scope block.
          }
          // std::cout<<"update tag"<<std::endl;
          // std::cout<<"Sf Interleaver inside "<< static_cast<int>(m_sf) <<std::endl;
          // Assign m_framelen_tag.value to pmt::from_long(8 + std::max((int)std::ceil((double)(m_frame_len - m_sf + 2) / (m_sf-2*m_ldro)) * (m_cr + 4), 0)). (inline comment notes: get number of items in frame)
          m_framelen_tag.value = pmt::from_long(8 + std::max((int)std::ceil((double)(m_frame_len - m_sf + 2) / (m_sf-2*m_ldro)) * (m_cr + 4), 0)); //get number of items in frame
          // Assign m_framelen_tag.offset to nitems_written(0).
          m_framelen_tag.offset = nitems_written(0); 
        // Close the current scope block.
        }
      // Close the current scope block.
      }

      

      // nitems_to_process = std::min(nitems_to_process)
      // handle the first interleaved block special case
      // Assign uint8_t cw_len to 4 + (((int)cw_cnt < m_sf - 2) ? 4 : m_cr).
      uint8_t cw_len = 4 + (((int)cw_cnt < m_sf - 2) ? 4 : m_cr);
      // Assign uint8_t sf_app to (((int)cw_cnt < m_sf - 2) ||m_ldro) ? m_sf - 2 : m_sf.
      uint8_t sf_app = (((int)cw_cnt < m_sf - 2) ||m_ldro) ? m_sf - 2 : m_sf;

      // Assign nitems_to_process to std::min(nitems_to_process,(int)sf_app).
      nitems_to_process = std::min(nitems_to_process,(int)sf_app);
      // Branch when condition (std::floor((float)noutput_items/cw_len)==0) evaluates to true.
      if(std::floor((float)noutput_items/cw_len)==0)
      // Open a new scope block.
      {
        // Return 0 to the caller.
        return 0;
      // Close the current scope block.
      }

      // Branch when condition (nitems_to_process >= sf_app || cw_cnt + nitems_to_process == (uint32_t)m_frame_len) evaluates to true.
      if (nitems_to_process >= sf_app || cw_cnt + nitems_to_process == (uint32_t)m_frame_len)
      // Open a new scope block.
      {        
        //propagate tag
        // Branch when condition (!cw_cnt) evaluates to true.
        if(!cw_cnt){
          // Call add_item_tag with arguments (0, m_framelen_tag).
          add_item_tag(0, m_framelen_tag);
          // Branch when condition (m_has_config_tag) evaluates to true.
          if(m_has_config_tag){
            // Call add_item_tag with arguments (0, m_config_tag).
            add_item_tag(0, m_config_tag);
            // Assign m_has_config_tag to false.
            m_has_config_tag = false;
          // Close the current scope block.
          }

        // Close the current scope block.
        }
        //Create the empty matrices
        // Execute statement std::vector<std::vector<bool>> cw_bin(sf_app).
        std::vector<std::vector<bool>> cw_bin(sf_app);
        // Execute statement std::vector<bool> init_bit(m_sf, 0).
        std::vector<bool> init_bit(m_sf, 0);
        // Execute statement std::vector<std::vector<bool>> inter_bin(cw_len, init_bit).
        std::vector<std::vector<bool>> inter_bin(cw_len, init_bit);

        //convert to input codewords to binary vector of vector
        // Iterate with loop parameters (int i = 0; i < sf_app; i++).
        for (int i = 0; i < sf_app; i++)
        // Open a new scope block.
        {
          // Branch when condition (i >= nitems_to_process) evaluates to true. (inline comment notes: ninput_items[0]))
          if (i >= nitems_to_process)//ninput_items[0])
            // Assign cw_bin[i] to int2bool(0, cw_len).
            cw_bin[i] = int2bool(0, cw_len);
          // Handle the alternative branch when previous conditions fail.
          else
            // Assign cw_bin[i] to int2bool(in[i], cw_len).
            cw_bin[i] = int2bool(in[i], cw_len);
          // Increment cw_cnt.
          cw_cnt++;
        // Close the current scope block.
        }

// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
        // Declare std::cout << "codewords---- " << std::endl.
        std::cout << "codewords---- " << std::endl;
        // Iterate with loop parameters (uint32_t i = 0u; i < sf_app; i++).
        for (uint32_t i = 0u; i < sf_app; i++)
        // Open a new scope block.
        {
          // Iterate with loop parameters (int j = 0; j < int(cw_len); j++).
          for (int j = 0; j < int(cw_len); j++)
          // Open a new scope block.
          {
            // Declare std::cout << cw_bin[i][j].
            std::cout << cw_bin[i][j];
          // Close the current scope block.
          }
          // Execute statement std::cout << " 0x" << std::hex << (int)in[i] << std::dec << std::endl.
          std::cout << " 0x" << std::hex << (int)in[i] << std::dec << std::endl;
        // Close the current scope block.
        }
        // Declare std::cout << std::endl.
        std::cout << std::endl;
// Close the preceding conditional compilation block.
#endif
        //Do the actual interleaving
        // Iterate with loop parameters (int32_t i = 0; i < cw_len; i++).
        for (int32_t i = 0; i < cw_len; i++)
        // Open a new scope block.
        {
          // Iterate with loop parameters (int32_t j = 0; j < int(sf_app); j++).
          for (int32_t j = 0; j < int(sf_app); j++)
          // Open a new scope block.
          {
            // Assign inter_bin[i][j] to cw_bin[mod((i - j - 1), sf_app)][i].
            inter_bin[i][j] = cw_bin[mod((i - j - 1), sf_app)][i];
          // Close the current scope block.
          }
          //For the first bloc we add a parity bit and a zero in the end of the lora symbol(reduced rate)
          // Branch when condition (((int)cw_cnt == m_sf - 2)||m_ldro) evaluates to true.
          if (((int)cw_cnt == m_sf - 2)||m_ldro)
            // Assign inter_bin[i][sf_app] to accumulate(inter_bin[i].begin(), inter_bin[i].end(), 0) % 2.
            inter_bin[i][sf_app] = accumulate(inter_bin[i].begin(), inter_bin[i].end(), 0) % 2;

          // Assign out[i] to bool2int(inter_bin[i]).
          out[i] = bool2int(inter_bin[i]);
        // Close the current scope block.
        }

// Only compile the following section when GRLORA_DEBUG is defined.
#ifdef GRLORA_DEBUG
        // Declare std::cout << "interleaved------" << std::endl.
        std::cout << "interleaved------" << std::endl;
        // Iterate with loop parameters (uint32_t i = 0u; i < cw_len; i++).
        for (uint32_t i = 0u; i < cw_len; i++)
        // Open a new scope block.
        {
          // Iterate with loop parameters (int j = 0; j < int(m_sf); j++).
          for (int j = 0; j < int(m_sf); j++)
          // Open a new scope block.
          {
            // Declare std::cout << inter_bin[i][j].
            std::cout << inter_bin[i][j];
          // Close the current scope block.
          }
          // Declare std::cout << " " << out[i] << std::endl.
          std::cout << " " << out[i] << std::endl;
        // Close the current scope block.
        }
        // Declare std::cout << std::endl.
        std::cout << std::endl;
// Close the preceding conditional compilation block.
#endif
        // Call consume_each with arguments (nitems_to_process > sf_app ? sf_app : nitems_to_process).
        consume_each(nitems_to_process > sf_app ? sf_app : nitems_to_process);
        // Return cw_len to the caller.
        return cw_len;
      // Close the current scope block.
      }
      // Handle the alternative branch when previous conditions fail.
      else
        // Return 0 to the caller.
        return 0;
    // Close the current scope block.
    }

  // Close the current scope and emit the trailing comment.
  } /* namespace lora */
// Close the current scope and emit the trailing comment.
} /* namespace gr */
