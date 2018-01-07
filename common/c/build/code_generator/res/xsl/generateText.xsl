<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format"
	xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:fn="http://www.w3.org/2005/xpath-functions"
	xmlns:navbuilder="http://www.navbuilder.com">
	<xsl:output method="text" omit-xml-declaration="yes" indent="yes" />
	<xsl:strip-space elements="*" />

	<xsl:template match="/">
		<xsl:apply-templates />
	</xsl:template>

	<xsl:template match="package">
		<xsl:apply-templates />
	</xsl:template>

	<xsl:template match="type">
		<xsl:text> - </xsl:text>
		<xsl:value-of select="@wireName" />
		<xsl:text>: [</xsl:text>
		<xsl:apply-templates />
		<xsl:text>]</xsl:text>
		<xsl:text>&#xa;</xsl:text>
	</xsl:template>

	<xsl:template match="field[@isSimple = 'yes']">
		<xsl:value-of select="@wireName" />
		<xsl:if test="position() != last()">
			<xsl:text>, </xsl:text>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>