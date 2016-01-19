make clean
make

./trace trace_files_9_9_15/ArpTest.pcap > myOut.out
diff myOut.out trace_files_9_9_15/ArpTest.out

./trace trace_files_9_9_15/IP_bad_checksum.pcap > myOut.out
diff myOut.out trace_files_9_9_15/IP_bad_checksum.out

./trace trace_files_9_9_15/largeMix2.pcap > myOut.out
diff myOut.out trace_files_9_9_15/largeMix2.out

./trace trace_files_9_9_15/largeMix.pcap > myOut.out
diff myOut.out trace_files_9_9_15/largeMix.out

./trace trace_files_9_9_15/PingTest.pcap > myOut.out
diff myOut.out trace_files_9_9_15/PingTest.out

./trace trace_files_9_9_15/smallTCP.pcap > myOut.out
diff myOut.out trace_files_9_9_15/smallTCP.out

./trace trace_files_9_9_15/TCP_bad_checksum.pcap > myOut.out
diff myOut.out trace_files_9_9_15/TCP_bad_checksum.out

./trace trace_files_9_9_15/UDPfile.pcap > myOut.out
diff myOut.out trace_files_9_9_15/UDPfile.out












