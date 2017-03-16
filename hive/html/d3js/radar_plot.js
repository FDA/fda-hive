/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*
        var d = [
                  [
                    {axis:"Email",value:0.59},
                    {axis:"Social Networks",value:0.56},
                    {axis:"Internet Banking",value:0.42},
                    {axis:"News Sportsites",value:0.34},
                    {axis:"Search Engine",value:0.48},
                    {axis:"View Shopping sites",value:0.14},
                    {axis:"Paying Online",value:0.11},
                    {axis:"Buy Online",value:0.05},
                    {axis:"Stream Music",value:0.07},
                    {axis:"Online Gaming",value:0.12},
                    {axis:"Navigation",value:0.27},
                    {axis:"App connected to TV program",value:0.03},
                    {axis:"Offline Gaming",value:0.12},
                    {axis:"Photo Video",value:0.4},
                    {axis:"Reading",value:0.03},
                    {axis:"Listen Music",value:0.22},
                    {axis:"Watch TV",value:0.03},
                    {axis:"TV Movies Streaming",value:0.03},
                    {axis:"Listen Radio",value:0.07},
                    {axis:"Sending Money",value:0.18},
                    {axis:"Other",value:0.07},
                    {axis:"Use less Once week",value:0.08}
                  ],[
                    {axis:"Email",value:0.48},
                    {axis:"Social Networks",value:0.41},
                    {axis:"Internet Banking",value:0.27},
                    {axis:"News Sportsites",value:0.28},
                    {axis:"Search Engine",value:0.46},
                    {axis:"View Shopping sites",value:0.29},
                    {axis:"Paying Online",value:0.11},
                    {axis:"Buy Online",value:0.14},
                    {axis:"Stream Music",value:0.05},
                    {axis:"Online Gaming",value:0.19},
                    {axis:"Navigation",value:0.14},
                    {axis:"App connected to TV program",value:0.06},
                    {axis:"Offline Gaming",value:0.24},
                    {axis:"Photo Video",value:0.17},
                    {axis:"Reading",value:0.15},
                    {axis:"Listen Music",value:0.12},
                    {axis:"Watch TV",value:0.1},
                    {axis:"TV Movies Streaming",value:0.14},
                    {axis:"Listen Radio",value:0.06},
                    {axis:"Sending Money",value:0.16},
                    {axis:"Other",value:0.07},
                    {axis:"Use less Once week",value:0.17}
                  ]
                ];*/

//var d;
function vjD3JS_RadarView ( viewer )
{
    vjD3CategoryView.call(this,viewer); // inherit default behaviours of the DataViewer

    this.width=700;
    this.height=700;
    if(!this.maxContributors)this.maxContributors=24;

    
    this.d3Compose=function(data){
            
        this.d3Compose_prv(data);
        
        if (data.length < 1) return;
        
        var thiSS=this;
        var svg=this.d3svg;//area.append("svg");
        
        var options=this.options;
        
        
        var cfg = {
            radius: 5,
            w: this.width,
            h: this.height,
            factor: 1,
            factorLegend: .85,
            levels: 10,
            maxValue: 0,
            radians: 2 * Math.PI,
            opacityArea: 0.5,
            ToRight: 5,
            TranslateX: 40,
            TranslateY: 30,
            ExtraWidthX: 100,
            ExtraWidthY: 60,
            thiSS:thiSS,
            color: function (series){
                //a=pastelColor( this.thiSS.categoryColors[(this.thiSS.colorshift+series)%this.thiSS.categoryColors.length],(1/(1.+series*series)));//d3.scale.category10(series);
                //a=lightenColor (this.thiSS.categoryColors[(this.thiSS.colorshift+series)%this.thiSS.categoryColors.length],0*(1-(1/(1.0001+series))));
                //a=this.thiSS.categoryColors[(this.thiSS.colorshift+series)%this.thiSS.categoryColors.length];
                a=this.thiSS.categoryColors[(this.thiSS.colorshift+series)%this.thiSS.categoryColors.length];
                return a;
            }
            //,color: d3.scale.category10(series)
            //color: function(ic,scale){shadeColor(this.categoryColors[(this.colorshift+ic)%this.categoryColors.length],scale)}  //d3.scale.category10()
        };
        var d=[];
        cfg.maxValue=0;
        if((typeof thiSS.sortOn!=='undefined') && (thiSS.sortOn!=null) ){
            data.sort(function(a, b) {
                if( a[thiSS.traits[thiSS.sortOn]]<b[thiSS.traits[thiSS.sortOn]]) {
                    return 1;
                }
                if( a[thiSS.traits[thiSS.sortOn]]>b[thiSS.traits[thiSS.sortOn]]) {
                    return -1;
                }
                return 0;
            });
        }
        for(var t=0; t<this.traits.length; ++t ) {
            if(!d[t]){ d[t]=[];}
            
            var cnt=data.length < this.maxContributors ? data.length : this.maxContributors;
            var tscale=0;
            for ( var l=0; l<cnt; ++l) {
                var val=parseFloat(data[l][this.traits[t]]);
                val*=val;
                if(tscale<val)tscale=val;
            }if(!tscale)tscale=1;
            
            
            for ( var l=0; l<cnt; ++l) {
                //var k= parseInt((l%2))==0 ? parseInt(l/2) : parseInt(cnt/2)+parseInt(l/2);
                var k = l;
                    
                if(!d[t][k])d[t][k]={}
                var val=parseFloat(data[l][this.traits[t]]); // Math.abs(
                val*=val;
                
                d[t][k]['axis']=data[l][this.id];//this.traits[t];
                d[t][k]['value']=val/tscale;
            }
        }
        
        
        
        if('undefined' !== typeof options){
          for(var i in options){
            if('undefined' !== typeof options[i]){
              cfg[i] = options[i];
            }
          }
        }
        cfg.maxValue = Math.max(
                cfg.maxValue, d3.max(d, function(i){
                    return d3.max(
                        i.map(
                            function(o){
                                return o.value;
                            }
                        )
                    )
                }
            )
        );
        var allAxis = (d[0].map(function(i, j){return i.axis}));
        var total = allAxis.length;
        var radius = cfg.factor*Math.min(cfg.w/2, cfg.h/2);
        var Format = d3.format('%');
        //svg.remove();
        
        var g =  svg.attr("width", cfg.w+cfg.ExtraWidthX)
                .attr("height", cfg.h+cfg.ExtraWidthY)
                .append("g")
                .attr("transform", "translate(" + cfg.TranslateX + "," + cfg.TranslateY + ")");
                ;
     
        var tooltip;
        
        //Circular segments
        for(var j=0; j<cfg.levels-1; j++){
          var levelFactor = cfg.factor*radius*((j+1)/cfg.levels);
          g.selectAll(".levels")
           .data(allAxis)
           .enter()
           .append("svg:line")
           .attr("x1", function(d, i){return levelFactor*(1-cfg.factor*Math.sin(i*cfg.radians/total));})
           .attr("y1", function(d, i){return levelFactor*(1-cfg.factor*Math.cos(i*cfg.radians/total));})
           .attr("x2", function(d, i){return levelFactor*(1-cfg.factor*Math.sin((i+1)*cfg.radians/total));})
           .attr("y2", function(d, i){return levelFactor*(1-cfg.factor*Math.cos((i+1)*cfg.radians/total));})
           .attr("class", "line")
           .style("stroke", "grey")
           .style("stroke-opacity", "0.75")
           .style("stroke-width", "0.3px")
           .attr("transform", "translate(" + (cfg.w/2-levelFactor) + ", " + (cfg.h/2-levelFactor) + ")");
        }
     
        //Text indicating at what % each level is
        for(var j=0; j<cfg.levels; j++){
          var levelFactor = cfg.factor*radius*((j+1)/cfg.levels);
          g.selectAll(".levels")
           .data([1]) //dummy data
           .enter()
           .append("svg:text")
           .attr("x", function(d){return levelFactor*(1-cfg.factor*Math.sin(0));})
           .attr("y", function(d){return levelFactor*(1-cfg.factor*Math.cos(0));})
           .attr("class", "legend")
           .style("font-family", "sans-serif")
           .style("font-size", "10px")
           .attr("transform", "translate(" + (cfg.w/2-levelFactor + cfg.ToRight) + ", " + (cfg.h/2-levelFactor) + ")")
           .attr("fill", "#737373")
           .text(Format((j+1)*cfg.maxValue/cfg.levels));
        }
        
        series = 0;
     
        var axis = g.selectAll(".axis")
                .data(allAxis)
                .enter()
                .append("g")
                .attr("class", "axis");
     
        axis.append("line")
            .attr("x1", cfg.w/2)
            .attr("y1", cfg.h/2)
            .attr("x2", function(d, i){return cfg.w/2*(1-cfg.factor*Math.sin(i*cfg.radians/total));})
            .attr("y2", function(d, i){return cfg.h/2*(1-cfg.factor*Math.cos(i*cfg.radians/total));})
            .attr("class", "line")
            .style("stroke", "grey")
            .style("stroke-width", "1px");
     
        axis.append("text")
            .attr("class", "legend")
            .text(function(d){return d;}
            )
            .style("font-family", "sans-serif")
            .style("font-size", "11px")
            .attr("text-anchor", "middle")
            .attr("dy", "1.5em")
            .attr("transform", function(d, i){return "translate(0, -10)"})
            //.attr("x", function(d, i){return 0;})
            //.attr("y", function(d, i){return 0;})
            //.attr("color", 'black')
            .attr("x", function(d, i){return cfg.w/2*(1-cfg.factorLegend*Math.sin(i*cfg.radians/total))-60*Math.sin(i*cfg.radians/total);})
            .attr("y", function(d, i){return cfg.h/2*(1-Math.cos(i*cfg.radians/total))-20*Math.cos(i*cfg.radians/total);})
        ;
     
     
        d.forEach(function(y, x){
          dataValues = [];
          g.selectAll(".nodes")
            .data(y, function(j, i){
              dataValues.push([
                cfg.w/2*(1-(parseFloat(Math.max(j.value, 0))/cfg.maxValue)*cfg.factor*Math.sin(i*cfg.radians/total)), 
                cfg.h/2*(1-(parseFloat(Math.max(j.value, 0))/cfg.maxValue)*cfg.factor*Math.cos(i*cfg.radians/total))
              ]);
            });
          dataValues.push(dataValues[0]);
          g.selectAll(".area")
                         .data([dataValues])
                         .enter()
                         .append("polygon")
                         .attr("class", "radar-chart-serie"+series)
                         .style("stroke-width", "2px")
                         .style("stroke", cfg.color(series))
                         //shadeColor(color, shade, base )
                         .attr("points",function(d) {
                             var str="";
                             for(var pti=0;pti<d.length;pti++){
                                 str=str+d[pti][0]+","+d[pti][1]+" ";
                             }
                             return str;
                          })
                         .style("fill", function(j, i){return cfg.color(series)})
                         .style("fill-opacity", cfg.opacityArea)
                         .on('mouseover', function (d){
                                            z = "polygon."+d3.select(this).attr("class");
                                            g.selectAll("polygon")
                                             .transition(200)
                                             .style("fill-opacity", 0.1); 
                                            g.selectAll(z)
                                             .transition(200)
                                             .style("fill-opacity", .7);
                                          })
                         .on('mouseout', function(){
                                            g.selectAll("polygon")
                                             .transition(200)
                                             .style("fill-opacity", cfg.opacityArea);
                         });
          series++;
        });
        series=0;
     
     
        d.forEach(function(y, x){
          g.selectAll(".nodes")
            .data(y).enter()
            .append("svg:circle")
            .attr("class", "radar-chart-serie"+series)
            .attr('r', cfg.radius)
            .attr("alt", function(j){return Math.max(j.value, 0)})
            .attr("cx", function(j, i){
              dataValues.push([
                cfg.w/2*(1-(parseFloat(Math.max(j.value, 0))/cfg.maxValue)*cfg.factor*Math.sin(i*cfg.radians/total)), 
                cfg.h/2*(1-(parseFloat(Math.max(j.value, 0))/cfg.maxValue)*cfg.factor*Math.cos(i*cfg.radians/total))
            ]);
            return cfg.w/2*(1-(Math.max(j.value, 0)/cfg.maxValue)*cfg.factor*Math.sin(i*cfg.radians/total));
            })
            .attr("cy", function(j, i){
              return cfg.h/2*(1-(Math.max(j.value, 0)/cfg.maxValue)*cfg.factor*Math.cos(i*cfg.radians/total));
            })
            .attr("data-id", function(j){return j.axis})
            .style("fill", cfg.color(series)).style("fill-opacity", .9)
            .on('mouseover', function (d){
                        newX =  parseFloat(d3.select(this).attr('cx')) - 10;
                        newY =  parseFloat(d3.select(this).attr('cy')) - 5;
                        
                        tooltip
                            .attr('x', newX)
                            .attr('y', newY)
                            .text(Format(d.value))
                            .transition(200)
                            .style('opacity', 1);
                            
                        z = "polygon."+d3.select(this).attr("class");
                        g.selectAll("polygon")
                            .transition(200)
                            .style("fill-opacity", 0.1); 
                        g.selectAll(z)
                            .transition(200)
                            .style("fill-opacity", .7);
                      })
            .on('mouseout', function(){
                        tooltip
                            .transition(200)
                            .style('opacity', 0);
                        g.selectAll("polygon")
                            .transition(200)
                            .style("fill-opacity", cfg.opacityArea);
                      })
            .append("svg:title")
            .text(function(j){return Math.max(j.value, 0)});
     
          series++;
        });
        //Tooltip
        tooltip = g.append('text')
                   .style('opacity', 0)
                   .style('font-family', 'sans-serif')
                   .style('font-size', '13px');
    }
    
    
}





function vjRadarPanelView(viewer)
{
    vjPanelView.call(this, viewer);
    this.iconSize=32;
    this.showTitles=true;

    this.rows=verarr(this.rows).concat(
            [
             {
                 name : 'starCnt',
                 path : '/starCnt',
                 type : 'text',
                 forceUpdate : true,
                 size : '7',
                 prefix : 'star',
                 align : 'right',
                 description : 'Defines the number of nodes shown on radar',
                 order : '1',
                 value : '12'
                 //url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(urlExchangeParameter(url,'readsAsFasta','1'),'down','1'))"
             },
             {
                 name : 'sortOn',
                 path : '/sortOn',
                 type : 'text',
                 forceUpdate : true,
                 size : '7',
                 prefix : 'sort',
                 align : 'right',
                 description : 'Defines the sorting order of radar vector',
                 order : '2',
                 value : '0'
                 //url : "javascript:var url='$(dataurl)';if(url.indexOf('?cmd')==-1)alert('nothing to download'); else funcLink(urlExchangeParameter(urlExchangeParameter(url,'readsAsFasta','1'),'down','1'))"
             },{
                     name:'apply', 
                     title: 'Apply' ,
                     order:2, 
                     icon:'refresh' ,
                     align :'right', 
                     description: 'refresh the content of the control with updated parameters' ,  
                     url: "javascript:vjObjEvent(\"onApplyStar\",\""+this.objCls+"\",'apply');"
             }
             ]
        );

        this.onApplyStar=function( container, param) 
        {
            this.radarObject.maxContributors=this.findElement("starCnt").value;
            this.radarObject.sortOn=this.findElement("sortOn").value;
            this.radarObject.refresh();
        }

}







function vjD3JS_RadarControl(viewer)
{
    
    this.radarViewer = new vjD3JS_RadarView ( viewer );
    if(viewer.noToolbar)return  [this.radarViewer];
    
    this.panelViewer = new vjRadarPanelView({
        data: "dsVoid",
        width:this.width,
        formObject: document.forms[viewer.formName],// viewer.formObject,
        //formName: viewer.formName,
        radarObject:this.radarViewer
    });
  
    return [this.panelViewer,this.radarViewer];
}

