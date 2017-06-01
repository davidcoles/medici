#ifdef __cplusplus
extern "C" {
#endif

void xmlnprint (char *, int);
void xmlprint (char *);
void xmlprintn (const char *, int);
void xmlcdata (char *);
void vxmlstartelement (char *, ...);
void vxmlvoidelement (char *, ...);
void vxmlprocessinginstruction (char *, ...);
void vxmlelement (char *, char *, ...);
void vxmlcomment (char *, ...);

void xmlattributes (char **);
void xmlelement (char *, char **, char *);
void xmlstartelement (char *, char **);
void xmlendelement (char *);
void xmlvoidelement (char *, char **);
void xmlprocessinginstruction (char *, char **);

#ifdef __cplusplus
}
#endif
