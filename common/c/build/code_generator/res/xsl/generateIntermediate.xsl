<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format"
	xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:fn="http://www.w3.org/2005/xpath-functions"
	xmlns:navbuilder="http://www.navbuilder.com">
	<xsl:output method="xml" omit-xml-declaration="no" indent="yes" />
	<xsl:strip-space elements="*" />

  
  <xsl:template match="/inlined">
    <xsl:element name="intermediate">
      <xsl:copy-of select="@*" />
      <xsl:apply-templates />
    </xsl:element>
  </xsl:template>

  <xsl:template match="type">
		<xsl:element name="type">
			<xsl:choose>
				<xsl:when test="@wireName">
					<xsl:copy-of select="@wireName" />
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="wireName" select="@name" />
				</xsl:otherwise>
			</xsl:choose>
			<xsl:choose>
				<xsl:when test="@targetName">
					<xsl:copy-of select="@targetName" />
				</xsl:when>
			</xsl:choose>
			<xsl:choose>
				<xsl:when test="@verificationItem">
					<xsl:copy-of select="@verificationItem" />
				</xsl:when>
			</xsl:choose>
			<xsl:choose>
				<xsl:when test="@description">
					<xsl:copy-of select="@description" />
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="description" select="@name" />
				</xsl:otherwise>
			</xsl:choose>
			<xsl:copy-of select="@package | @name | @isAbstract | @base | @discriminator-value" />

			<xsl:variable name="uniqueTypes" as="item()*"
				select="element[not(@name = following::rerefence/@name)]" />
			<xsl:for-each-group select="$uniqueTypes"
				group-by="@name">
				<xsl:apply-templates select="current-group()"
					mode="generateImport" />
			</xsl:for-each-group>

			<xsl:choose>
				<xsl:when test="@isAbstract = 'yes'">
					<xsl:variable name="type" select="@name" />
					<xsl:apply-templates select="/inlined/type[@base = $type]"
						mode="generateSubtypes" />
				</xsl:when>
			</xsl:choose>

			<xsl:apply-templates />
		</xsl:element>
	</xsl:template>

	<xsl:template match="type" mode="generateSubtypes">
		<xsl:element name="subType">
			<xsl:attribute name="type" select="@name" />
			<xsl:attribute name="package" select="@package" />
		</xsl:element>

	</xsl:template>

	<xsl:template match="handler">
		<xsl:copy-of select="current()" />
	</xsl:template>

	<xsl:template match="discriminator">
		<xsl:element name="discriminator">
			<xsl:copy-of select="@*" />
		</xsl:element>
	</xsl:template>

	<xsl:template match="element" mode="generateImport">
		<xsl:variable name="type" select="@type" />
		<xsl:variable name="types" as="item()?"
			select="/inlined/type[@name = $type]" />
		<xsl:if test="count($types) > 0">
			<xsl:element name="import">
				<xsl:attribute name="package" select="$types[1]/@package" />
				<xsl:attribute name="type" select="$types[1]/@name" />
			</xsl:element>
		</xsl:if>
	</xsl:template>

 
  <xsl:template match="element | attribute">
    <xsl:element name="field">
			<xsl:copy-of select="@name | @type | @isArray | @default | @forVerification | @isOptional" />
			<xsl:choose>
				<xsl:when test="string-length(@wireName) > 0">
					<xsl:attribute name="wireName" select="@wireName" />
				</xsl:when>
				<xsl:when test="name(.) = 'element'">
					<xsl:variable name="type" select="@type" />
					<xsl:variable name="typeElement" select="/inlined/type[@name = $type]" />
					<xsl:attribute name="wireName" select="$typeElement/@wireName" />
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="wireName" select="@name" />
				</xsl:otherwise>
			</xsl:choose>
			<xsl:choose>
				<xsl:when test="string-length(@targetName) > 0">
					<xsl:attribute name="targetName" select="@targetName" />
				</xsl:when>
				<xsl:when test="name(.) = 'element'">
					<xsl:variable name="type" select="@type" />
					<xsl:variable name="typeElement" select="/inlined/type[@name = $type]" />
					<xsl:attribute name="targetName" select="$typeElement/@targetName" />
				</xsl:when>
			</xsl:choose>
			<xsl:choose>
				<xsl:when test="string-length(@verificationItem) > 0">
					<xsl:attribute name="verificationItem" select="@verificationItem" />
				</xsl:when>
				<xsl:when test="name(.) = 'element'">
					<xsl:variable name="type" select="@type" />
					<xsl:variable name="typeElement" select="/inlined/type[@name = $type]" />
					<xsl:attribute name="verificationItem" select="$typeElement/@verificationItem" />
				</xsl:when>
			</xsl:choose>

			<xsl:choose>
				<xsl:when test="string-length(@description) > 0">
					<xsl:attribute name="description" select="@description" />
				</xsl:when>
				<xsl:when test="name(.) = 'element'">
					<xsl:variable name="type" select="@type" />
					<xsl:variable name="typeElement" select="/inlined/type[@name = $type]" />
					<xsl:attribute name="description" select="$typeElement/@description" />
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="description" select="@name" />
				</xsl:otherwise>
			</xsl:choose>

			<xsl:choose>
				<xsl:when test="name(.) = 'element'">
					<xsl:attribute name="isSimple" select="'no'" />
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="isSimple" select="'yes'" />
				</xsl:otherwise>
			</xsl:choose>

		</xsl:element>
<xsl:for-each select="field">
    <xsl:sort select="@name"/>
  </xsl:for-each>
	</xsl:template>
  

</xsl:stylesheet>