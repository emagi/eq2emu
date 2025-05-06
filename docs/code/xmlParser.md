# File: `xmlParser.h`

## Classes

- `XMLNodeContents`
- `XMLNodeDataTag`
- `XMLNodeDataTag`

## Functions

- `XMLSTR createXMLString(int nFormat=1, int *pnSize=NULL) const;`
- `XMLCSTR getName() const;                                       ///< name of the node`
- `XMLCSTR getText(int i=0) const;                                ///< return ith text field`
- `int nText() const;                                             ///< nbr of text field`
- `XMLNode getParentNode() const;                                 ///< return the parent node`
- `XMLNode getChildNode(int i=0) const;                           ///< return ith child node`
- `XMLNode getChildNode(XMLCSTR name, int i)  const;              ///< return ith child node with specific name (return an empty node if failing). If i==-1, this returns the last XMLNode with the given name.`
- `XMLNode getChildNode(XMLCSTR name, int *i=NULL) const;         ///< return next child node with specific name (return an empty node if failing)`
- `XMLNode getChildNodeByPath(XMLCSTR path, char createNodeIfMissing=0, XMLCHAR sep='/');`
- `XMLNode getChildNodeByPathNonConst(XMLSTR  path, char createNodeIfMissing=0, XMLCHAR sep='/');`
- `int nChildNode(XMLCSTR name) const;                            ///< return the number of child node with specific name`
- `int nChildNode() const;                                        ///< nbr of child node`
- `XMLAttribute getAttribute(int i=0) const;                      ///< return ith attribute`
- `XMLCSTR      getAttributeName(int i=0) const;                  ///< return ith attribute name`
- `XMLCSTR      getAttributeValue(int i=0) const;                 ///< return ith attribute value`
- `char  isAttributeSet(XMLCSTR name) const;                      ///< test if an attribute with a specific name is given`
- `XMLCSTR getAttribute(XMLCSTR name, int i) const;               ///< return ith attribute content with specific name (return a NULL if failing)`
- `XMLCSTR getAttribute(XMLCSTR name, int *i=NULL) const;         ///< return next attribute content with specific name (return a NULL if failing)`
- `int nAttribute() const;                                        ///< nbr of attribute`
- `XMLClear getClear(int i=0) const;                              ///< return ith clear field (comments)`
- `int nClear() const;                                            ///< nbr of clear field`
- `XMLNodeContents enumContents(XMLElementPosition i) const;      ///< enumerate all the different contents (attribute,child,text, clear) of the current XMLNode. The order is reflecting the order of the original file/string. NOTE: 0 <= i < nElement();`
- `int nElement() const;                                          ///< nbr of different contents for current node`
- `char isEmpty() const;                                          ///< is this node Empty?`
- `char isDeclaration() const;                                    ///< is this node a declaration <? .... ?>`
- `XMLNode deepCopy() const;                                      ///< deep copy (duplicate/clone) a XMLNode`
- `XMLNode        addChild(XMLCSTR lpszName, char isDeclaration=FALSE, XMLElementPosition pos=-1); ///< Add a new child node`
- `XMLNode        addChild(XMLNode nodeToAdd, XMLElementPosition pos=-1);                          ///< If the "nodeToAdd" has some parents, it will be detached from it's parents before being attached to the current XMLNode`
- `XMLCSTR        addText(XMLCSTR lpszValue, XMLElementPosition pos=-1);                           ///< Add a new text content`
- `XMLCSTR       updateName(XMLCSTR lpszName);                                                  ///< change node's name`
- `XMLCSTR       updateText(XMLCSTR lpszNewValue, int i=0);                                     ///< if the text to update is missing, a new one will be added`
- `XMLCSTR       updateText(XMLCSTR lpszNewValue, XMLCSTR lpszOldValue);                        ///< if the text to update is missing, a new one will be added`
- `void deleteNodeContent();`
- `void deleteAttribute(int i=0);                   ///< Delete the ith attribute of the current XMLNode`
- `void deleteAttribute(XMLCSTR lpszName);          ///< Delete the attribute with the given name (the "strcmp" function is used to find the right attribute)`
- `void deleteAttribute(XMLAttribute *anAttribute); ///< Delete the attribute with the name "anAttribute->lpszName" (the "strcmp" function is used to find the right attribute)`
- `void deleteText(int i=0);                        ///< Delete the Ith text content of the current XMLNode`
- `void deleteText(XMLCSTR lpszValue);              ///< Delete the text content "lpszValue" inside the current XMLNode (direct "pointer-to-pointer" comparison is used to find the right text)`
- `void deleteClear(int i=0);                       ///< Delete the Ith clear tag inside the current XMLNode`
- `void deleteClear(XMLCSTR lpszValue);             ///< Delete the clear tag "lpszValue" inside the current XMLNode (direct "pointer-to-pointer" comparison is used to find the clear tag)`
- `void deleteClear(XMLClear *p);                   ///< Delete the clear tag "p" inside the current XMLNode (direct "pointer-to-pointer" comparison on the lpszName of the clear tag is used to find the clear tag)`
- `XMLNode        addChild_WOSD(XMLSTR lpszName, char isDeclaration=FALSE, XMLElementPosition pos=-1);  ///< Add a new child node`
- `XMLCSTR        addText_WOSD(XMLSTR lpszValue, XMLElementPosition pos=-1);                            ///< Add a new text content`
- `XMLCSTR        updateName_WOSD(XMLSTR lpszName);                                                  ///< change node's name`
- `XMLCSTR        updateText_WOSD(XMLSTR lpszNewValue, int i=0);                                     ///< if the text to update is missing, a new one will be added`
- `XMLCSTR        updateText_WOSD(XMLSTR lpszNewValue, XMLCSTR lpszOldValue);                        ///< if the text to update is missing, a new one will be added`
- `XMLElementPosition positionOfText(int i=0) const;`
- `XMLElementPosition positionOfText(XMLCSTR lpszValue) const;`
- `XMLElementPosition positionOfClear(int i=0) const;`
- `XMLElementPosition positionOfClear(XMLCSTR lpszValue) const;`
- `XMLElementPosition positionOfClear(XMLClear *a) const;`
- `XMLElementPosition positionOfChildNode(int i=0) const;`
- `XMLElementPosition positionOfChildNode(XMLNode x) const;`
- `XMLElementPosition positionOfChildNode(XMLCSTR name, int i=0) const; ///< return the position of the ith childNode with the specified name if (name==NULL) return the position of the ith childNode`
- `char parseClearTag(void *px, void *pa);`
- `char maybeAddTxT(void *pa, XMLCSTR tokenPStr);`
- `int ParseXMLElement(void *pXML);`
- `int indexText(XMLCSTR lpszValue) const;`
- `int indexClear(XMLCSTR lpszValue) const;`
- `XMLNode addChild_priv(int,XMLSTR,char,int);`
- `XMLCSTR addText_priv(int,XMLSTR,int);`
- `void emptyTheNode(char force);`
- `void freeBuffer();///<call this function when you have finished using this object to release memory used by the internal buffer.`
- `XMLSTR toXML(XMLCSTR source);///< returns a pointer to an internal buffer that contains a XML-encoded string based on the "source" parameter.`
- `void freeBuffer();///< Call this function when you have finished using this object to release memory used by the internal buffer.`
- `XMLSTR encode(unsigned char *inByteBuf, unsigned int inByteLen, char formatted=0); ///< returns a pointer to an internal buffer containing the base64 string containing the binary data encoded from "inByteBuf"`
- `void alloc(int newsize);`

## Notable Comments

- /****************************************************************************/
- /*! \mainpage XMLParser library
- * \section intro_sec Introduction
- *
- * This is a basic XML parser written in ANSI C++ for portability.
- * It works by using recursion and a node tree for breaking
- * down the elements of an XML document.
- *
- * @version     V2.44
- * @author      Frank Vanden Berghen
