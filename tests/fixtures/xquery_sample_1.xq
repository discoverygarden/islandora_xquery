(: Name xquery_sample_1.xq :)
xquery version "3.0" encoding "UTF-8";
declare namespace mods="http://www.loc.gov/mods/v3";
copy $input := .
modify (
  for $text in $input//text()
  return replace value of node $text with fn:upper-case($text)
)
return $input
