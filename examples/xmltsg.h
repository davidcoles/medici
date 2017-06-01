#ifdef __cplusplus
extern "C" {
#endif

  extern char *xmlbuff;
  extern char *pyxbuff;

  
  /* xmltsg.c */
  EDI_Directory read_xmltsg_file();
  EDI_Directory read_xmltsg_buffer();
  EDI_Directory read_xmltsg_stream();
  EDI_Directory read_xmltsg_fd();
  EDI_Directory read_pyxtsg_file();
  EDI_Directory read_pyxtsg_buffer();
  EDI_Directory read_pyxtsg_stream();
  EDI_Directory read_pyxtsg_fd();


#ifdef __cplusplus
}
#endif
