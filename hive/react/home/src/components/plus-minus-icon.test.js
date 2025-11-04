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
import React from 'react';
import { render, screen, fireEvent } from '@testing-library/react';
import '@testing-library/jest-dom';
import PlusMinusIcon from './plus-minus-icon';

// Mock the Icon component from antd
jest.mock('antd', () => ({
  Icon: ({ type, onClick }) => (
    <div data-testid="icon" onClick={onClick}>
      {type}
    </div>
  ),
}));

describe('PlusMinusIcon Component', () => {
  test('renders the plus icon when isActive is true', () => {
    render(<PlusMinusIcon isActive={true} setIsActive={jest.fn()} />);
    //console.log(screen.getByText('plus'));
    expect(screen.getByText('plus')).toBeInTheDocument();
  });

  test('renders the minus icon when isActive is false', () => {
    render(<PlusMinusIcon isActive={false} setIsActive={jest.fn()} />);
    expect(screen.getByText('minus')).toBeInTheDocument();
  });

  test('calls setIsActive with the correct argument on icon click', () => {
    const setIsActiveMock = jest.fn();
    render(<PlusMinusIcon isActive={true} setIsActive={setIsActiveMock} />);

    // Find and click the plus icon
    const icon = screen.getByText('plus');
    fireEvent.click(icon);

    // Check if setIsActive was called with the expected argument
    expect(setIsActiveMock).toHaveBeenCalledWith(false);
  });
});
