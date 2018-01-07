<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format"
	xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:fn="http://www.w3.org/2005/xpath-functions"
	xmlns:navbuilder="http://www.navbuilder.com">
	<xsl:output method="xml" omit-xml-declaration="no" indent="yes" />
	<xsl:strip-space elements="*" />

	<xsl:template match="type">
		<xsl:variable name="type" select="." />
		<xsl:variable name="namePrefix" select="substring-before(@name, '{')" />
		<xsl:variable name="namePostfix" select="substring-after(@name, '}')" />
		<xsl:variable name="wireNamePrefix" select="substring-before(@wireName, '{')" />
		<xsl:variable name="wireNamePostfix" select="substring-after(@wireName, '}')" />

		<xsl:variable name="list"
			select="substring-before(substring-after(@name, '{'), '}')" />
		<xsl:variable name="wireList"
			select="substring-before(substring-after(@wireName, '{'), '}')" />
		<xsl:if test="string-length($list) > 0">
			<xsl:variable name="tokenizedList" select="tokenize($list, ',')" />
			<xsl:variable name="tokenizedWireList" select="tokenize($wireList, ',')" />

			<xsl:call-template name="typeCopy">
				<xsl:with-param name="type" select="$type" />
				<xsl:with-param name="namePrefix" select="$namePrefix" />
				<xsl:with-param name="namePostfix" select="$namePostfix" />
				<xsl:with-param name="nameList" select="$tokenizedList" />
				<xsl:with-param name="wireNamePrefix" select="$wireNamePrefix" />
				<xsl:with-param name="wireNamePostfix" select="$wireNamePostfix" />
				<xsl:with-param name="wireNameList" select="$tokenizedWireList" />
			</xsl:call-template>
		</xsl:if>
		<xsl:if test="string-length($list) = 0">
			<xsl:copy>
				<xsl:apply-templates select="@* | node()" />
			</xsl:copy>
		</xsl:if>
	</xsl:template>

	<xsl:template name="typeCopy">
		<xsl:param name="type" />
		<xsl:param name="namePrefix" />
		<xsl:param name="namePostfix" />
		<xsl:param name="nameList" />
		<xsl:param name="wireNamePrefix" />
		<xsl:param name="wireNamePostfix" />
		<xsl:param name="wireNameList" />
		<xsl:if test="$nameList[1]">
			<xsl:element name="type">
				<xsl:apply-templates select="$type/@*" />
				<xsl:attribute name="name"
					select="concat(concat($namePrefix, normalize-space($nameList[1])), $namePostfix)" />
				<xsl:attribute name="wireName"
					select="concat(concat($wireNamePrefix, normalize-space($wireNameList[1])), $wireNamePostfix)" />
				<xsl:apply-templates select="$type/node()" />
			</xsl:element>
			<xsl:call-template name="typeCopy">
				<xsl:with-param name="type" select="$type" />
				<xsl:with-param name="namePrefix" select="$namePrefix" />
				<xsl:with-param name="namePostfix" select="$namePostfix" />
				<xsl:with-param name="nameList" select="$nameList[position() > 1]" />
				<xsl:with-param name="wireNamePrefix" select="$wireNamePrefix" />
				<xsl:with-param name="wireNamePostfix" select="$wireNamePostfix" />
				<xsl:with-param name="wireNameList" select="$wireNameList[position() > 1]" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

<xsl:template match="element">
		<xsl:variable name="element" select="." />
		<xsl:variable name="namePrefix" select="substring-before(@name, '{')" />
		<xsl:variable name="namePostfix" select="substring-after(@name, '}')" />
		<xsl:variable name="typePrefix" select="substring-before(@type, '{')" />
		<xsl:variable name="typePostfix" select="substring-after(@type, '}')" />

		<xsl:variable name="list"
			select="substring-before(substring-after(@name, '{'), '}')" />
		<xsl:variable name="wireList"
			select="substring-before(substring-after(@type, '{'), '}')" />
		<xsl:if test="string-length($list) > 0">
			<xsl:variable name="tokenizedList" select="tokenize($list, ',')" />
			<xsl:variable name="tokenizedWireList" select="tokenize($wireList, ',')" />

			<xsl:call-template name="elementCopy">
				<xsl:with-param name="element" select="$element" />
				<xsl:with-param name="namePrefix" select="$namePrefix" />
				<xsl:with-param name="namePostfix" select="$namePostfix" />
				<xsl:with-param name="nameList" select="$tokenizedList" />
				<xsl:with-param name="typePrefix" select="$typePrefix" />
				<xsl:with-param name="typePostfix" select="$typePostfix" />
				<xsl:with-param name="typeList" select="$tokenizedWireList" />
			</xsl:call-template>
		</xsl:if>
		<xsl:if test="string-length($list) = 0">
			<xsl:copy>
				<xsl:apply-templates select="@* | node()" />
			</xsl:copy>
		</xsl:if>
	</xsl:template>
	
	<xsl:template name="elementCopy">
		<xsl:param name="element" />
		<xsl:param name="namePrefix" />
		<xsl:param name="namePostfix" />
		<xsl:param name="nameList" />
		<xsl:param name="typePrefix" />
		<xsl:param name="typePostfix" />
		<xsl:param name="typeList" />
		<xsl:if test="$nameList[1]">
			<xsl:element name="element">
				<xsl:apply-templates select="$element/@*" />
				<xsl:attribute name="name"
					select="concat(concat($namePrefix, normalize-space($nameList[1])), $namePostfix)" />
				<xsl:attribute name="type"
					select="concat(concat($typePrefix, normalize-space($typeList[1])), $typePostfix)" />
				<xsl:apply-templates select="$element/node()" />
			</xsl:element>
			<xsl:call-template name="elementCopy">
				<xsl:with-param name="element" select="$element" />
				<xsl:with-param name="namePrefix" select="$namePrefix" />
				<xsl:with-param name="namePostfix" select="$namePostfix" />
				<xsl:with-param name="nameList" select="$nameList[position() > 1]" />
				<xsl:with-param name="typePrefix" select="$typePrefix" />
				<xsl:with-param name="typePostfix" select="$typePostfix" />
				<xsl:with-param name="typeList" select="$typeList[position() > 1]" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>
	
	<xsl:template match="/ | @* | node()">
		<xsl:copy>
			<xsl:apply-templates select="@* | node()" />
		</xsl:copy>
	</xsl:template>


</xsl:stylesheet>