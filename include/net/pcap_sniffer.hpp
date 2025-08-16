#pragma once

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>
#include <pcap.h>
#include <nlohmann/json.hpp>

namespace environet {
namespace net {

/**
 * @brief Packet metadata structure
 * 
 * Contains essential information about captured packets
 */
struct PacketMeta {
    uint64_t timestamp_ms;      // Timestamp in milliseconds
    uint32_t length;            // Packet length in bytes
    std::string src_mac;        // Source MAC address
    std::string dst_mac;        // Destination MAC address
    uint16_t ethertype;         // Ethernet type
    std::string src_ip;         // Source IP address (if available)
    std::string dst_ip;         // Destination IP address (if available)
    uint16_t src_port;          // Source port (if TCP/UDP)
    uint16_t dst_port;          // Destination port (if TCP/UDP)
    uint8_t protocol;           // IP protocol number
    int signal_strength;        // Signal strength in dBm (if radiotap available)
    int noise_level;            // Noise level in dBm (if radiotap available)
    
    // Default constructor
    PacketMeta() : timestamp_ms(0), length(0), ethertype(0), src_port(0), 
                   dst_port(0), protocol(0), signal_strength(0), noise_level(0) {}
};

/**
 * @brief PCAP packet sniffer class
 * 
 * Provides packet capture capabilities with BPF filtering and file rotation
 */
class PcapSniffer {
public:
    /**
     * @brief Packet callback function type
     */
    using PacketCallback = std::function<void(const PacketMeta&, const uint8_t*)>;
    
    /**
     * @brief Constructor
     * 
     * @param config_path Path to configuration file
     */
    explicit PcapSniffer(const std::string& config_path);
    
    /**
     * @brief Destructor
     */
    ~PcapSniffer();
    
    /**
     * @brief Initialize packet capture
     * 
     * @return true if successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Start packet capture
     * 
     * @param callback Function to call for each captured packet
     * @return true if successful, false otherwise
     */
    bool start(PacketCallback callback);
    
    /**
     * @brief Stop packet capture
     */
    void stop();
    
    /**
     * @brief Check if capture is running
     * 
     * @return true if capture is active
     */
    bool is_running() const { return running_; }
    
    /**
     * @brief Get capture statistics
     * 
     * @return JSON object with capture statistics
     */
    nlohmann::json get_stats() const;
    
    /**
     * @brief Get last error message
     * 
     * @return Error message string
     */
    std::string get_last_error() const { return last_error_; }

private:
    // Configuration
    std::string interface_;
    std::string bpf_filter_;
    std::string output_dir_;
    size_t max_file_size_mb_;
    int max_files_;
    bool promiscuous_;
    
    // PCAP state
    pcap_t* pcap_handle_;
    pcap_dumper_t* pcap_dumper_;
    std::string current_pcap_file_;
    std::vector<std::string> file_history_;
    std::atomic<int> file_index_{0};
    int datalink_ = -1;
    
    // Capture state
    std::atomic<bool> running_;
    std::thread capture_thread_;
    PacketCallback packet_callback_;
    
    // Statistics
    uint64_t packets_captured_;
    uint64_t packets_dropped_;
    uint64_t bytes_captured_;
    uint64_t start_time_ms_;
    
    // Error handling
    std::string last_error_;
    
    // Private methods
    bool init_interface();
    bool compile_bpf();
    bool open_pcap_file();
    void close_pcap_file();
    void rotate_pcap_file();
    
    /**
     * @brief Main capture loop
     */
    void capture_loop();
    
    /**
     * @brief Process a captured packet
     * 
     * @param header PCAP packet header
     * @param packet Packet data
     */
    void process_packet(const pcap_pkthdr* header, const uint8_t* packet);
    
    /**
     * @brief Parse Ethernet header
     * 
     * @param packet Packet data
     * @param meta Packet metadata to fill
     * @return true if parsing successful
     */
    bool parse_ethernet_header(const uint8_t* packet, PacketMeta& meta);
    
    /**
     * @brief Parse IP header
     * 
     * @param packet Packet data (starting at IP header)
     * @param meta Packet metadata to fill
     * @return true if parsing successful
     */
    bool parse_ip_header(const uint8_t* packet, PacketMeta& meta);
    
    /**
     * @brief Parse TCP header
     * 
     * @param packet Packet data (starting at TCP header)
     * @param meta Packet metadata to fill
     * @return true if parsing successful
     */
    bool parse_tcp_header(const uint8_t* packet, PacketMeta& meta);
    
    /**
     * @brief Parse UDP header
     * 
     * @param packet Packet data (starting at UDP header)
     * @param meta Packet metadata to fill
     * @return true if parsing successful
     */
    bool parse_udp_header(const uint8_t* packet, PacketMeta& meta);
    
    /**
     * @brief Parse radiotap header (if available)
     * 
     * @param packet Packet data (starting at radiotap header)
     * @param meta Packet metadata to fill
     * @return true if parsing successful
     */
    bool parse_radiotap_header(const uint8_t* packet, PacketMeta& meta);
    
    /**
     * @brief Set error message
     * 
     * @param error Error message
     */
    void set_error(const std::string& error);
    
    /**
     * @brief Clean up PCAP resources
     */
    void cleanup();
    
    /**
     * @brief Get current timestamp in milliseconds
     * 
     * @return Current timestamp
     */
    static uint64_t get_current_time_ms();
    
    /**
     * @brief Convert MAC address bytes to string
     * 
     * @param mac MAC address bytes
     * @return MAC address string
     */
    static std::string mac_to_string(const uint8_t* mac);
    
    /**
     * @brief Convert IP address bytes to string
     * 
     * @param ip IP address bytes
     * @param version IP version (4 or 6)
     * @return IP address string
     */
    static std::string ip_to_string(const uint8_t* ip, int version);
};

} // namespace net
} // namespace environet
