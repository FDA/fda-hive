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
import React, { useState, useEffect  } from 'react';
import { Link, useLocation } from 'react-router-dom';
import * as UrlModal from '../hivelib/modal/url_modal';

function Navbar() {
    const [activeIndex, setActiveIndex] = useState(null);
    //const [hoverIndex, setHoverIndex] = useState(null);
    const location = useLocation();
    useEffect(() => {
        if (/tab=about/.test(location.search)) {
            setActiveIndex(0);
        }
    }, [location]);

    const handleMouseEnter = index => {
        //setHoverIndex(index);
    };

    const handleMouseLeave = () => {
        //setHoverIndex(null);
    };

    const handleItemClick = index => {
        setActiveIndex(index);
    };

    return (
        <nav style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', padding: '0.0rem 1rem' }}>
            <div style={{ display: 'flex', alignItems: 'center', marginTop: '10px' }}>
                <div style={{ marginRight: '80px' }}>

                </div>
                <ul style={{ listStyleType: 'none', display: 'flex', margin: 0, padding: 0 }}>
                    <li className="menu-item" onClick={() => handleItemClick(0)} onMouseEnter={() => handleMouseEnter('about')}
                        onMouseLeave={handleMouseLeave} style={{ position: 'relative' }}>
                        <Link to="?tab=about" style={{ textDecoration: 'none', color: activeIndex === 0 ? '#ff9504' : '#5a6772' }}>
                            <img height="24" src={`${UrlModal.getPrefixPlain()}/img/HIVE_logo_transparent_small.png`}
                                style={{ marginRight: '3px', marginBottom: '3px' }} alt='' ></img>
                            About HIVE
                        </Link>
                    </li>
                    <li className="menu-item" onClick={() => handleItemClick(1)} onMouseEnter={() => handleMouseEnter('tutorials')}
                        onMouseLeave={handleMouseLeave} style={{ position: 'relative' }}>
                        <a href={`${UrlModal.getPrefix()}?cmd=main-tutorials`} style={{ textDecoration: 'none', color: activeIndex === 1 ? '#ff9504' : '#5a6772' }}>
                            <img height="24" src={`${UrlModal.getPrefixPlain()}/img/help.gif`}
                                style={{ marginRight: '3px', marginBottom: '3px' }} alt='' ></img>
                            Tutorials
                        </a>
                    </li>
                    <li className="menu-item" onClick={() => handleItemClick(2)} onMouseEnter={() => handleMouseEnter('events')}
                        onMouseLeave={handleMouseLeave} style={{ position: 'relative' }}>
                        <a href={`${UrlModal.getPrefix()}?cmd=main-events`} style={{ textDecoration: 'none', color: activeIndex === 2 ? '#ff9504' : '#5a6772' }}>
                            <img height="24" src={`${UrlModal.getPrefixPlain()}/img/calendar-img.png`}
                                style={{ marginRight: '3px', marginBottom: '3px' }} alt='' ></img>
                            Events </a>
                    </li>

                    <li className="menu-item" onClick={() => handleItemClick(3)}>
                        <a href={`${UrlModal.getPrefix()}?cmd=main-publications`} style={{ textDecoration: 'none', color: activeIndex === 3 ? '#ff9504' : '#5a6772' }}>
                            <img height="24" src={`${UrlModal.getPrefixPlain()}/img/publication.png`}
                                style={{ marginRight: '3px', marginBottom: '3px' }} alt='' ></img>
                                Publications
                        </a>
                    </li>
                    <li className="menu-item" onClick={() => handleItemClick(4)}>
                        <a href={`${UrlModal.getPrefix()}?cmd=main-people`} style={{ textDecoration: 'none', color: activeIndex === 4 ? '#ff9504' : '#5a6772' }}>
                        <img height="24" src={`${UrlModal.getPrefixPlain()}/img/user-group.gif`}
                                style={{ marginRight: '3px', marginBottom: '3px' }} alt='' ></img>
                                People
                        </a>
                    </li>
                </ul>
            </div>
        </nav>
    );
}

export default Navbar;