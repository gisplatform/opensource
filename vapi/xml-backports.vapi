/*
 * xml-backports.vapi is a file with missing libxml2 VAPI elements.
 *
 * Copyright (C) 2018 Sergey Volkhin.
 *
 * xml-backports.vapi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xml-backports.vapi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xml-backports.vapi. If not, see <http://www.gnu.org/licenses/>.
 *
*/

[CCode (cheader_filename = "libxml/xmlwriter.h")]
namespace XmlBackports
{
  namespace TextWriter
  {
    #if VALA_0_34
      [Version(deprecated = true, deprecated_since = "0.34", replacement = "Xml.TextWriter.doc")]
    #endif
    [CCode (cname = "xmlNewTextWriterDoc")]
    public static Xml.TextWriter doc(out Xml.Doc doc, bool compression = false);

    #if VALA_0_34
      [Version(deprecated = true, deprecated_since = "0.34", replacement = "Xml.TextWriter.memory")]
    #endif
    [CCode (cname = "xmlNewTextWriterMemory")]
    public static Xml.TextWriter memory(Xml.Buffer buffer, bool compression = false);

    #if VALA_0_34
      [Version(deprecated = true, deprecated_since = "0.34", replacement = "Xml.TextWriter.parser")]
    #endif
    [CCode (cname = "xmlNewTextWriterPushParser")]
    public static Xml.TextWriter parser(Xml.ParserCtxt ctxt, bool compression = false);

    #if VALA_0_34
      [Version(deprecated = true, deprecated_since = "0.34", replacement = "Xml.TextWriter.tree")]
    #endif
    [CCode (cname = "xmlNewTextWriterTree")]
    public static Xml.TextWriter tree(Xml.Doc doc, Xml.Node node, bool compression = false);
  }
}

