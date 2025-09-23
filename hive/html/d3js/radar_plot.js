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

function vjD3JS_RadarView ( viewer )
{
    vjD3CategoryView.call(this,viewer);

    this.width=700;
    this.height=700;
    this.nonSticky = false;
    if(!this.maxContributors)this.maxContributors=24;
    if(this.allTraits=="all")this.allTraits=undefined;

    
    this.d3Compose=function(data){
            
        this.d3Compose_prv(data);
        
        if (data.length < 1) return;
        
        var thiSS=this;
        var svg=this.d3svg;
        
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
                a=this.thiSS.categoryColors[(this.thiSS.colorshift+series)%this.thiSS.categoryColors.length];
                return a;
            }
        };
        var d=[];
        cfg.maxValue=0;
        if((typeof thiSS.sortOn!=='undefined') && (thiSS.sortOn!=null) ){
            data.sort(function(a, b) {
                if( Math.abs(a[thiSS.traits[thiSS.sortOn]])<Math.abs(b[thiSS.traits[thiSS.sortOn]])) {
                    return 1;
                }
                if( Math.abs(a[thiSS.traits[thiSS.sortOn]])>Math.abs(b[thiSS.traits[thiSS.sortOn]])) {
                    return -1;
                }
                return 0;
            });
        }
        
        var shownTraits= (this.allTraits && this.allTraits!="all" ) ? this.allTraits.split(",") : undefined ; 
        
        for(var t=0, T; t<this.traits.length ; ++t ) {
            if(shownTraits){
                
                if(t>=shownTraits.length)
                    break;
                else 
                    T=parseInt(shownTraits[t]);
            }else T=t;
                
            
            
            if(!d[t]){ d[t]=[];}
            
            var cnt=data.length < this.maxContributors ? data.length : this.maxContributors;
            var tscale=0;
            for ( var l=0; l<cnt; ++l) {
                var val=parseFloat(data[l][this.traits[T]]);
                val*=val;
                if(tscale<val)tscale=val;
            }if(!tscale)tscale=1;
            if(cfg.thiSS.seriesScales  && cfg.thiSS.seriesScales[t]  ) 
                  tscale/=cfg.thiSS.seriesScales[t];
            
            
            for ( var l=0; l<cnt; ++l) {
                var k = l;
                    
                if(!d[t][k])d[t][k]={}
                var val=parseFloat(data[l][this.traits[T]]);
                val*=val;
            
                
                  
                d[t][k]['axis']=data[l][this.id];
                d[t][k]['value']=val/tscale;
                cfg.maxValue = Math.max(cfg.maxValue,d[t][k]['value']);
            }
        }
        
        
        
        if('undefined' !== typeof options){
          for(var i in options){
            if('undefined' !== typeof options[i]){
              cfg[i] = options[i];
            }
          }
        }
        if (d.length < 1) return;
        
        var dims = Math.min (cfg.w-cfg.ExtraWidthX, cfg.h-cfg.ExtraWidthY);
        
        var allAxis = (d[0].map(function(i, j){return i.axis}));
        var total = allAxis.length;
        var radius = cfg.factor*dims/2;
        var Format = d3.format('%');
        
        
        var g =  svg.attr("width", dims+100)
                .attr("height", dims+100)
                .append("g")
                .attr("transform", "translate(" + cfg.TranslateX + "," + cfg.TranslateY + ")");
                ;
     
        var tooltip;
        
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
           .attr("transform", "translate(" + (dims/2-levelFactor) + ", " + (dims/2-levelFactor) + ")");
        }
     
        for(var j=0; j<cfg.levels; j++){
          var levelFactor = cfg.factor*radius*((j+1)/cfg.levels);
          g.selectAll(".levels")
           .data([1])
           .enter()
           .append("svg:text")
           .attr("x", function(d){return levelFactor*(1-cfg.factor*Math.sin(0));})
           .attr("y", function(d){return levelFactor*(1-cfg.factor*Math.cos(0));})
           .attr("class", "legend")
           .style("font-family", "sans-serif")
           .style("font-size", "10px")
           .attr("transform", "translate(" + (dims/2-levelFactor + cfg.ToRight) + ", " + (dims/2-levelFactor) + ")")
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
            .attr("x1", dims/2)
            .attr("y1", dims/2)
            .attr("x2", function(d, i){return dims/2*(1-cfg.factor*Math.sin(i*cfg.radians/total));})
            .attr("y2", function(d, i){return dims/2*(1-cfg.factor*Math.cos(i*cfg.radians/total));})
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
            .attr("x", function(d, i){return dims/2*(1-cfg.factorLegend*Math.sin(i*cfg.radians/total))-60*Math.sin(i*cfg.radians/total);})
            .attr("y", function(d, i){return dims/2*(1-Math.cos(i*cfg.radians/total))-20*Math.cos(i*cfg.radians/total);})
        ;
     
     
        d.forEach(function(y, x){
          dataValues = [];
          g.selectAll(".nodes")
            .data(y, function(j, i){
              dataValues.push([
                dims/2*(1-(parseFloat(Math.max(j.value, 0))/cfg.maxValue)*cfg.factor*Math.sin(i*cfg.radians/total)), 
                dims/2*(1-(parseFloat(Math.max(j.value, 0))/cfg.maxValue)*cfg.factor*Math.cos(i*cfg.radians/total))
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
                dims/2*(1-(parseFloat(Math.max(j.value, 0))/cfg.maxValue)*cfg.factor*Math.sin(i*cfg.radians/total)), 
                dims/2*(1-(parseFloat(Math.max(j.value, 0))/cfg.maxValue)*cfg.factor*Math.cos(i*cfg.radians/total))
            ]);
            var rrr=dims/2*(1-(Math.max(j.value, 0)/cfg.maxValue)*cfg.factor*Math.sin(i*cfg.radians/total));
            return rrr;
            })
            .attr("cy", function(j, i){
              var rrr=dims/2*(1-(Math.max(j.value, 0)/cfg.maxValue)*cfg.factor*Math.cos(i*cfg.radians/total));
              return rrr;
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
                 value : '36'
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
             },{
                 name : 'allTraits',
                 path : '/allTraits',
                 type : 'text',
                 forceUpdate : true,
                 size : '7',
                 prefix : 'traits',
                 align : 'right',
                 description : 'Defines the number of traits to be shown',
                 order : '2',
                 value : viewer.radarObject.allTraits ? viewer.radarObject.allTraits : "all"
             },{
                 name : 'traitsScales',
                 path : '/traitsScales',
                 type : 'text',
                 forceUpdate : true,
                 size : '7',
                 prefix : 'scales',
                 align : 'right',
                 description : 'Defines the scaling factor ',
                 order : '2',
                 value : viewer.radarObject.traitsScales ? viewer.radarObject.traitsScales : ' '
             },
             
             {
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
            this.radarObject.maxContributors=parseInt(this.findElement("starCnt").value);
            this.radarObject.sortOn=parseInt(this.findElement("sortOn").value);
            this.radarObject.allTraits=this.findElement("allTraits").value;
            this.radarObject.seriesScales=this.findElement("traitsScales").value.split(",");
            for(var i=0;i<this.radarObject.seriesScales.length; ++i)this.radarObject.seriesScales[i]=parseFloat(this.radarObject.seriesScales[i]);
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
        formObject: document.forms[viewer.formName],
        radarObject:this.radarViewer
    });
  
    return [this.panelViewer,this.radarViewer];
}

