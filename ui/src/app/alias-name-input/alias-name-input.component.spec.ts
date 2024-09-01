import { ComponentFixture, TestBed } from '@angular/core/testing';

import { AliasNameInputComponent } from './alias-name-input.component';

describe('AliasNameInputComponent', () => {
  let component: AliasNameInputComponent;
  let fixture: ComponentFixture<AliasNameInputComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ AliasNameInputComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(AliasNameInputComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
