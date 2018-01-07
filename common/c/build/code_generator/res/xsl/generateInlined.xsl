<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format"
	xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:fn="http://www.w3.org/2005/xpath-functions"
	xmlns:navbuilder="http://www.navbuilder.com">
	<xsl:output method="xml" omit-xml-declaration="no" indent="yes" />
	<xsl:strip-space elements="*" />

	<xsl:template match="/protocol">
		<xsl:element name="inlined">
			<xsl:copy-of select="@*"/>
			<xsl:apply-templates />
		</xsl:element>
	</xsl:template>

	<xsl:template match="package">
		<xsl:apply-templates />
	</xsl:template>

	<xsl:template match="handler">
		<xsl:variable name="name" select="@name" />
		<xsl:copy>
			<xsl:attribute name="name" select="@name" />
			<xsl:attribute name="package" select="../@name" />
			<xsl:apply-templates />
		</xsl:copy>
	</xsl:template>

	<xsl:template match="type">
		<xsl:for-each select="attribute">
		    <xsl:sort select="@name"/>
		  </xsl:for-each>
		<xsl:variable name="name" select="@name" />
		
		<!-- inline type definition -->
		<xsl:copy>
			<xsl:attribute name="name" select="@name" />
			<xsl:attribute name="package" select="../@name" />
			<xsl:attribute name="wireName" select="@wireName" />
			<xsl:attribute name="targetName" select="@targetName" />
			<xsl:attribute name="verificationItem" select="@verificationItem" />
			<xsl:attribute name="forVerification" select="@forVerification" />
			<xsl:attribute name="description" select="@description" />
			<xsl:attribute name="isAbstract"
				select="if (@isAbstract = 'yes') then 'yes' else 'no'" />
			<xsl:attribute name="base"
				select="if (@base) then @base else 'Element'" />
			<xsl:variable name="package"
				select="/protocol/package[type/@name=$name]/@name" />
			<xsl:if test="string-length($package) > 0">
				<xsl:attribute name="package" select="../@name" />
			</xsl:if>
			<xsl:copy-of select="@discriminator-value" />

			<!-- collect all the imports -->
			<xsl:variable name="uniqueElements" as="item()*"
				select="element[not(@name = following::rerefence/@name)]" />
			<xsl:for-each-group select="$uniqueElements"
				group-by="@name">

				<xsl:variable name="importType" select="@type" />
				<xsl:variable name="importPackage"
					select="/protocol/package[type/@name=$importType]/@name" />
				<xsl:choose>
					<xsl:when test="string-length($importPackage) > 0">
						<xsl:element name="import">
							<xsl:attribute name="package" select="$importPackage" />
							<xsl:attribute name="type" select="$importType" />
						</xsl:element>
					</xsl:when>
				</xsl:choose>

			</xsl:for-each-group>

			<xsl:apply-templates />
		</xsl:copy>
	</xsl:template>

	<xsl:template match="element | attribute">
		<xsl:variable name="type" select="@type" />
		<xsl:copy>
			<xsl:copy-of select="@name | @package | @type | @wireName | @targetName | @verificationItem | @forVerification | @default | @description"  />
			<xsl:choose>
				<xsl:when test="@max">
					<xsl:if test="@max = '1'">
						<xsl:attribute name="isOptional"
							select="if (@min = '0') then 'yes' else 'no'" />
						<xsl:attribute name="isArray" select="'no'" />
					</xsl:if>
					<xsl:if test="@max != '1'">
						<xsl:attribute name="max" select="@max" />
						<xsl:attribute name="isArray" select="'yes'" />
						<xsl:attribute name="isOptional" select="'no'" />
						<xsl:choose>
							<xsl:when test="@min">
								<xsl:attribute name="min" select="@min" />
							</xsl:when>
							<xsl:otherwise>
								<xsl:attribute name="min" select="'0'" />
							</xsl:otherwise>
						</xsl:choose>
					</xsl:if>
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="isOptional" select="'no'" />
					<xsl:choose>
						<xsl:when test="@min">
							<xsl:attribute name="isArray" select="'yes'" />
						</xsl:when>
						<xsl:otherwise>
							<xsl:attribute name="isArray" select="'no'" />
						</xsl:otherwise>
					</xsl:choose>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:copy>
	</xsl:template>

	<xsl:template match="discriminator">
		<xsl:copy>
			<xsl:copy-of select="@*" />
		</xsl:copy>
	</xsl:template>

	<xsl:template match="request">
		<xsl:variable name="type" select="@type" />
		<xsl:copy>
			<xsl:copy-of select="@*" />
			<xsl:attribute name="package"
				select="/protocol/package[type/@name = $type]/@name" />
			<xsl:attribute name="wireName"
				select="/protocol/package/type[@name = $type]/@wireName" />
			<xsl:attribute name="targetName"
				select="/protocol/package/type[@name = $type]/@targetName" />
			<xsl:attribute name="verificationItem"
				select="/protocol/package/type[@name = $type]/@verificationItem" />
			<xsl:attribute name="description"
				select="/protocol/package/type[@name = $type]/@description" />
			<xsl:apply-templates />
		</xsl:copy>
	</xsl:template>

	<xsl:template match="response">
		<xsl:variable name="type" select="@type" />
		<xsl:copy>
			<xsl:copy-of select="@*" />
			<xsl:attribute name="package"
				select="/protocol/package[type/@name = $type]/@name" />
			<xsl:attribute name="wireName"
				select="/protocol/package/type[@name = $type]/@wireName" />
			<xsl:attribute name="targetName"
				select="/protocol/package/type[@name = $type]/@targetName" />
			<xsl:attribute name="verificationItem"
				select="/protocol/package/type[@name = $type]/@verificationItem" />
			<xsl:attribute name="description"
				select="/protocol/package/type[@name = $type]/@description" />
		</xsl:copy>
	</xsl:template>


</xsl:stylesheet>