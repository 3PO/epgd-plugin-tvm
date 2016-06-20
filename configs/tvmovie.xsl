<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/TR/xhtml1/strict"
                xmlns:date="http://exslt.org/dates-and-times">

<xsl:variable name="mapping" select="document('tvmovie-category.xml')/mapping" />
<xsl:variable name="categorieMapping" select="$mapping/categories/category/contains" />
<xsl:variable name="curYear" select="substring(//Sendung/Beginn,1,4)" />
<xsl:variable name="dststart" select="concat($curYear,'-03-',32-date:day-in-week(concat($curYear,'-03-31')),'T02:00:00')"/>
<xsl:variable name="dstend" select="concat($curYear,'-10-',32-date:day-in-week(concat($curYear,'-10-31')),'T03:00:00')"/>


<xsl:template match="/">

<events>

    <xsl:for-each select="//Sendung">

      <xsl:if test="number(SendungID) = SendungID">

	<event>

	<xsl:attribute name="id"><xsl:value-of select="SendungID"/></xsl:attribute>

<!--
-->

	<starttime>
	    <xsl:call-template name="date2UTC"><xsl:with-param name="date" select="translate(Beginn,' ','T')"/></xsl:call-template>
	</starttime>

	<duration><xsl:value-of select="substring-before(Dauer,' ')*60"/></duration>

	<xsl:if test="string-length(VPS)"><vps><xsl:value-of select="date:seconds(translate(VPS,' ','T'))"/></vps></xsl:if>


	<title><xsl:value-of select="Titel"/></title>

	<xsl:if test="string-length(Originaltitel)"><shorttext><xsl:value-of select="Originaltitel"/></shorttext></xsl:if>

	<xsl:if test="string-length(Herstellungsjahr)"><year><xsl:value-of select="Herstellungsjahr"/></year></xsl:if>
	<xsl:if test="string-length(Herstellungsland)"><country><xsl:value-of select="Herstellungsland"/></country></xsl:if>


        <xsl:choose>
            <xsl:when test="Bewertung = 5">
                <tipp>GoldTipp</tipp>
            </xsl:when>
            <xsl:when test="Bewertung = 4">
                <tipp>TagesTipp</tipp>
            </xsl:when>
            <xsl:when test="Bewertung = 3">
                <tipp>TopTipp</tipp>
            </xsl:when>
        </xsl:choose>


	<xsl:if test="string-length(Bewertung) and Bewertung &gt; 0 and Bewertung &lt; 6"><numrating><xsl:value-of select="Bewertung"/></numrating></xsl:if>


        <xsl:choose>
            <xsl:when test="Bewertung = 5">
                <txtrating>Einer der besten Filme aller Zeiten</txtrating>
            </xsl:when>
            <xsl:when test="Bewertung = 4">
                <txtrating>Sehr empfehlenswert</txtrating>
            </xsl:when>
            <xsl:when test="Bewertung = 3">
                <txtrating>Empfehlenswert</txtrating>
            </xsl:when>
            <xsl:when test="Bewertung = 2">
                <txtrating>Eher durchschnittlich</txtrating>
            </xsl:when>
            <xsl:when test="Bewertung = 1">
                <txtrating>Eher uninteressant</txtrating>
            </xsl:when>
        </xsl:choose>


        <xsl:variable name="rating">
            <xsl:call-template name="Bewertungen">
                <xsl:with-param name="str" select="concat(Bewertungen,';')"/>
            </xsl:call-template>
        </xsl:variable>

        <xsl:if test="string-length($rating)">
            <rating>
                <xsl:value-of select="$rating" />
            </rating>
        </xsl:if>


        <xsl:call-template name="mapping">
            <xsl:with-param name="str" select="Keywords" />
            <xsl:with-param name="default" select="Kategorietext" />
        </xsl:call-template>

<!--
	<xsl:if test="string-length(Keywords)"><topic><xsl:value-of select="Keywords"/></topic></xsl:if>
-->

        <xsl:if test="string-length(Genre)">
            <xsl:choose>
                <xsl:when test="substring(Genre,string-length(Genre)-5) = 'sserie' and
                not(contains('aeiou',substring(Genre,string-length(Genre)-6,1)))">
                    <genre><xsl:value-of select="substring(Genre,1,string-length(Genre)-6)"/></genre>
                </xsl:when>
                <xsl:when test="substring(Genre,string-length(Genre)-4) = 'serie'">
                    <genre><xsl:value-of select="substring(Genre,1,string-length(Genre)-5)"/></genre>
                </xsl:when>
                <xsl:when test="substring(Genre,string-length(Genre)-5) = '-Serie'">
                    <genre><xsl:value-of select="substring(Genre,1,string-length(Genre)-6)"/></genre>
                </xsl:when>
                <xsl:when test="substring(Genre,string-length(Genre)-5) = 'sreihe' and
                    not(contains('aeiou',substring(Genre,string-length(Genre)-6,1)))">
                        <genre><xsl:value-of select="substring(Genre,1,string-length(Genre)-6)"/></genre>
                </xsl:when>
                <xsl:when test="substring(Genre,string-length(Genre)-4) = 'reihe'">
                    <genre><xsl:value-of select="substring(Genre,1,string-length(Genre)-5)"/></genre>
                </xsl:when>
                <xsl:otherwise>
                    <genre><xsl:value-of select="Genre"/></genre>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>


        <xsl:variable name="AUDIO">
            <xsl:if test="KzDolby=1"><xsl:text> Dolby</xsl:text></xsl:if>
            <xsl:if test="KzDolbySurround=1"><xsl:text> DolbySurround</xsl:text></xsl:if>
            <xsl:if test="KzDolbyDigital=1"><xsl:text> DolbyDigital</xsl:text></xsl:if>
            <xsl:if test="KzStereo=1"><xsl:text> Stereo</xsl:text></xsl:if>
            <xsl:if test="KzZweikanalton=1"><xsl:text> Zweikanal</xsl:text></xsl:if>
        </xsl:variable>


	<xsl:if test="string-length($AUDIO)"><audio><xsl:value-of select="normalize-space($AUDIO)"/></audio></xsl:if>


        <xsl:variable name="FLAGS">
            <xsl:if test="KzHDTV=1"><xsl:text> [HDTV]</xsl:text></xsl:if>
            <xsl:if test="Kz16zu9=1"><xsl:text> [16:9]</xsl:text></xsl:if>
            <xsl:if test="KzSchwarzweis=1"><xsl:text> [SchwarzWeiss]</xsl:text></xsl:if>
            <xsl:if test="KzUntertitel=1"><xsl:text> [Untertitel]</xsl:text></xsl:if>
            <xsl:if test="KzAudiodescription=1"><xsl:text> [Audiodeskription]</xsl:text></xsl:if>
            <xsl:if test="KzLive=1"><xsl:text> [Live]</xsl:text></xsl:if>
        </xsl:variable>

	<xsl:if test="string-length($FLAGS)"><flags><xsl:value-of select="normalize-space($FLAGS)"/></flags></xsl:if>


        <xsl:variable name="sr" select="Kurzkritik"/>

        <xsl:variable name="SHORTREVIEW">
          <xsl:if test="string-length($sr) and $sr != 'HDTV'">
            <xsl:value-of select="$sr"/>
          </xsl:if>
        </xsl:variable>

        <xsl:if test="string-length($SHORTREVIEW)"><shortreview><xsl:value-of select="$SHORTREVIEW"/></shortreview></xsl:if>


        <xsl:variable name="ld" select="Beschreibung"/>

        <xsl:variable name="LONGDESCRIPTION">
          <xsl:if test="string-length($ld)">
            <xsl:choose>
              <xsl:when test="contains(substring-after(substring($ld, string-length($ld)-45),'Dies ist die '),' Episode')">
                <xsl:value-of select="normalize-space(substring-before($ld,concat('Dies ist die ',substring-after(substring($ld, string-length($ld)-45),'Dies ist die '))))"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$ld"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:if>
        </xsl:variable>

	<xsl:if test="string-length($LONGDESCRIPTION)"><longdescription><xsl:value-of select="$LONGDESCRIPTION"/></longdescription></xsl:if>


        <xsl:variable name="sd0" select="KurzBeschreibung"/>

        <xsl:variable name="sd1">
          <xsl:if test="string-length($sd0) and $sd0 != 'HDTV'">
            <xsl:choose>
              <xsl:when test="substring-before($sd0, ' ') = 'HDTV,'">
                <xsl:value-of select="substring-after($sd0, ' ')"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="$sd0"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:if>
        </xsl:variable>

        <xsl:variable name="SHORTDESCRIPTION">
          <xsl:choose>
	    <xsl:when test="string-length($sd1) and string-length($SHORTREVIEW) and substring($sd1,1,string-length($SHORTREVIEW)) = $SHORTREVIEW">
	      <xsl:value-of select="substring-after(substring($sd1,string-length($SHORTREVIEW)),' ')"/>
	    </xsl:when>
    	    <xsl:otherwise>
	      <xsl:value-of select="$sd1"/>
    	    </xsl:otherwise>
          </xsl:choose>
        </xsl:variable>

	<xsl:if test="string-length($SHORTDESCRIPTION) and substring($SHORTDESCRIPTION,1,string-length($SHORTDESCRIPTION)-4) != substring($LONGDESCRIPTION,1,string-length($SHORTDESCRIPTION)-4)">
	  <shortdescription>
	    <xsl:value-of select="$SHORTDESCRIPTION"/>
	  </shortdescription>
	</xsl:if>


	<xsl:if test="string-length(FSK)"><parentalrating><xsl:value-of select="FSK"/></parentalrating></xsl:if>

        <xsl:if test="string-length(Darsteller)">
            <xsl:call-template name="group2tag">
                <xsl:with-param name="str">
                    <xsl:call-template name="grouping">
                        <xsl:with-param name="str" select="concat(Darsteller,';')"/>
                        <xsl:with-param name="values" select="'[Moderator][Moderation]:moderator;[Regie]:director;[Gast][Gaststar]:guest;[Drehbuch]:screenplay;[Musik]:music;[Produzent]:producer;[Kommentar]:commentator;[Kamera]:camera;[Schnitt])[Szenen-/Bühnenbild])[Produktionsfirma])[Autor])[Kostüm])[Buch])[Ausstattung])[Spezialeffekte])[Filmverleih])[Ton])[Casting])[Regie-Assistenz])[Spezialpersonen])[Stunts])[Verantwortlich])[Choreograph])[Komponist])[Redner])[Cameoauftritt]):other;:actor'"/>
                    </xsl:call-template>
                    <xsl:text>;</xsl:text>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:if>

	<xsl:variable name="BILDER">
	    <xsl:for-each select="Bilddateiname">
		<xsl:value-of select="substring-before(.,'.jpg')" />
	    <xsl:text> </xsl:text>
	    </xsl:for-each>
	</xsl:variable>

	<xsl:if test="string-length(normalize-space($BILDER))">
	    <images><xsl:value-of select="translate(normalize-space($BILDER),' ',',')"/></images>
	    <imagetype>jpg</imagetype>
	</xsl:if>

	</event>
      </xsl:if>

    </xsl:for-each>

</events>

</xsl:template>


<xsl:template name="Bewertungen">
    <xsl:param name="str" select="."/>
    <xsl:variable name="part" select="substring-before($str, ';')" />
    <xsl:variable name="value" select="substring('**********', 1, number(substring-after($part, '=')))" />
    <xsl:if test="string-length($value)">
        <xsl:value-of select="concat(' / ', substring-before($part, '='), ' ', $value)"/>
    </xsl:if>
    <xsl:variable name="rest" select="substring-after($str, ';')"/>
    <xsl:if test="string-length($rest)">
	<xsl:call-template name="Bewertungen">
        <xsl:with-param name="str" select="$rest"/>
	</xsl:call-template>
    </xsl:if>
</xsl:template>

<xsl:template name="mapping">
    <xsl:param name="str" select="." />
    <xsl:param name="default" select="." />
   <xsl:variable name="value" select="translate($str,'ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖU', 'abcdefghijklmnopqrstuvwxyzäöü')" />
   <xsl:variable name="map" select="$categorieMapping[contains($value, .)]/../@name" />    
    <xsl:choose>
        <xsl:when test="string-length($map)">
          <category><xsl:value-of select="$map" /></category>
       </xsl:when>
        <xsl:when test="string-length($default)">
          <category><xsl:value-of select="$default" /></category>
       </xsl:when>
    </xsl:choose>
</xsl:template>


<!-- grouping gruppiert einen string, indem die Eintrage mit ', ' hinter dem jeweiligen key, bzw. vor dem folgenden ; geschrieben werden
     str: ein ; separierter String mit einem keywort in Klammern (Am Ende muss auch jeweils ein ; stehen), zb: Moderator1 (Moderation);Moderator2 (Moderator);Schauspieler1 (Hauptdarsteller);Schauspieler2 (Nebendarsteller);Schauspieler3 (Name der Filmfigur);Kamerakind (Kamera);Der Produzent (Produzent);Assistent 1 (Regie);Musiker1 (Musik);Autor1 (Drehbuch);Autor2 (Drehbuch);Musiker2 (Musik)
     values: ein ; separierter string mit folgendem Syntax: [key][key])[key n]optionaler text;
     eine ) hinter dem key bedeutet, dass der ursprüngliche key wieder in Klammern dahinter geschrieben wird
     Alle nicht zuordenbare keys werden ans Ende gehängt
     Beispiel: [Moderator][Moderation];[Regie];[Kamera])[Schnitt])[Produzent]); -> [Moderator][Moderation], Moderator1, Moderator2;[Regie], Assistent 1;[Kamera])[Schnitt])[Produzent]), Kamerakind (Kamera), Der Produzent (Produzent);, Schauspieler1 (Hauptdarsteller), Schauspieler2 (Nebendarsteller), Schauspieler3 (Name der Filmfigur), Musiker1 (Musik), Autor1 (Drehbuch), Autor2 (Drehbuch), Musiker2 (Musik)
-->
<xsl:template name="grouping">
    <xsl:param name="str" />
    <xsl:param name="values" />

    <xsl:variable name="term" select="substring-before($str, ';')" />
    <xsl:variable name="rest" select="substring-after($str, ';')"/>
    <xsl:variable name="newValues">
        <xsl:choose>
            <xsl:when test="string-length($term)">
                <xsl:variable name="part" select="substring-before($term, ')')" />
                <xsl:variable name="key" select="concat('[', substring-after($part, '('), ']')" />
                <xsl:variable name="valPart" select="substring-after($values, $key)" />
                <xsl:choose>
                    <xsl:when test="string-length($valPart)">
                        <xsl:variable name="insPos" select="string-length($values) - string-length($valPart) + string-length(substring-before($valPart, ';') )"/>
                        <xsl:choose>
                            <xsl:when test="substring($valPart, 1 ,1) = ')'">
                                <xsl:value-of select="concat(substring($values, 1, $insPos), ', ', $part, ')',  substring($values, $insPos + 1))" />
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="concat(substring($values, 1, $insPos), ', ', substring-before($part, ' ('),  substring($values, $insPos + 1))" />
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="concat($values, ', ', $term)" />
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$values" />
            </xsl:otherwise>
        </xsl:choose>
    </xsl:variable>
    <xsl:choose>
        <xsl:when test="string-length($rest)">
            <xsl:call-template name="grouping">
                <xsl:with-param name="str" select="$rest"/>
                <xsl:with-param name="values" select="$newValues"/>
            </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="$newValues"/>
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>


<!-- group2tag erzeugt aus einen key-value string xml-tags
     str: ein ; separierter String mit :key, value Werten (Am Ende muss auch jeweils ein ; stehen)

     str: ein ; separierter string mit folgendem Syntax: optionaler text:tagname, value;
        der ':' ist erforderlich und bezeichnet den Begin des tagnamens
        Wenn der Wert leer ist, wird der tag NICHT geschrieben
        Beispiel: text:moderator, Moderator1, Moderator2;ignorierter tag:screenplay;:director, Assistent 1; -> <moderator>Moderator1, Moderator2</moderator><director>Assistent 1</director>
-->
<xsl:template name="group2tag">
    <xsl:param name="str" select="."/>
    <xsl:variable name="part" select="substring-after(substring-before($str, ';'), ':')" />
    <xsl:variable name="rest" select="substring-after($str, ';')"/>
    <xsl:variable name="val" select="substring-after($part, ', ')" />
    <xsl:if test="string-length($val)">
        <xsl:element name="{substring-before($part, ',')}">
            <xsl:value-of select="$val"/>
        </xsl:element>
    </xsl:if>
    <xsl:if test="string-length($rest)">
        <xsl:call-template name="group2tag">
            <xsl:with-param name="str" select="$rest"/>
        </xsl:call-template>
    </xsl:if>
</xsl:template>

<xsl:template name="date2UTC">
    <xsl:param name="date"/>
    <xsl:variable name="tz">
        <xsl:choose>
            <xsl:when test="date:seconds(date:difference($dststart,$date)) &gt;= 0 and date:seconds(date:difference($date,$dstend)) &gt;= 0">+01:00</xsl:when>
            <xsl:otherwise>+00:00</xsl:otherwise>
        </xsl:choose>
    </xsl:variable>
    <xsl:value-of select="date:seconds(concat($date,$tz))"/>
</xsl:template>

</xsl:stylesheet>
